import os
from pathlib import Path

# Must run before importing api.main below, which creates tables against this URL on import
os.environ.setdefault("FO_DATABASE_URL", "sqlite:///./.test_import_placeholder.db")

import pytest
from fastapi.testclient import TestClient
from sqlalchemy import create_engine, event
from sqlalchemy.orm import sessionmaker

from api.dependencies.config import conf
from api.dependencies.database import Base, enable_foreign_keys, get_db
from api.main import app

PROJECT_ROOT = Path(__file__).resolve().parents[2]
SAMPLE_MIDI = PROJECT_ROOT / "sample_midis" / "Bach_Cantate_BWV147.mid"


@pytest.fixture()
def db_session(tmp_path):
    engine = create_engine(f"sqlite:///{tmp_path}/test.db", connect_args={"check_same_thread": False})
    event.listen(engine, "connect", enable_foreign_keys)
    Base.metadata.create_all(engine)
    session_local = sessionmaker(autocommit=False, autoflush=False, bind=engine)
    session = session_local()

    try:
        yield session
    finally:
        session.close()


@pytest.fixture()
def client(db_session, tmp_path, monkeypatch):
    monkeypatch.setattr(conf, "upload_dir", str(tmp_path / "uploads"))

    def override_get_db():
        yield db_session

    app.dependency_overrides[get_db] = override_get_db
    yield TestClient(app)
    app.dependency_overrides.clear()


@pytest.fixture()
def auth_client(client):
    client.post("/auth/signup", json={"email": "test@example.com", "password": "correcthorse"})
    response = client.post("/auth/login", json={"email": "test@example.com", "password": "correcthorse"})
    token = response.json()["access_token"]
    client.headers.update({"Authorization": f"Bearer {token}"})
    return client
