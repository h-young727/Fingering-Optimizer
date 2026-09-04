from api.tests.conftest import SAMPLE_MIDI


def test_upload_without_auth_rejected(client):
    with SAMPLE_MIDI.open("rb") as f:
        response = client.post("/pieces", files={"file": ("song.mid", f, "audio/midi")})

    assert response.status_code == 401


def test_upload_wrong_extension_rejected(auth_client, tmp_path):
    fake_file = tmp_path / "notmidi.txt"
    fake_file.write_text("not a midi file")

    with fake_file.open("rb") as f:
        response = auth_client.post("/pieces", files={"file": ("notmidi.txt", f, "text/plain")})

    assert response.status_code == 400


def test_upload_and_list_piece(auth_client):
    with SAMPLE_MIDI.open("rb") as f:
        upload_response = auth_client.post("/pieces", files={"file": ("song.mid", f, "audio/midi")})

    assert upload_response.status_code == 201
    assert upload_response.json()["filename"] == "song.mid"

    list_response = auth_client.get("/pieces")
    assert list_response.status_code == 200
    assert len(list_response.json()) == 1


def test_pieces_isolated_per_user(client):
    client.post("/auth/signup", json={"email": "userA@example.com", "password": "correcthorse"})
    token_a = client.post("/auth/login", json={"email": "userA@example.com", "password": "correcthorse"}).json()["access_token"]

    with SAMPLE_MIDI.open("rb") as f:
        client.post(
            "/pieces", headers={"Authorization": f"Bearer {token_a}"}, files={"file": ("song.mid", f, "audio/midi")}
        )

    client.post("/auth/signup", json={"email": "userB@example.com", "password": "correcthorse"})
    token_b = client.post("/auth/login", json={"email": "userB@example.com", "password": "correcthorse"}).json()["access_token"]

    response_b = client.get("/pieces", headers={"Authorization": f"Bearer {token_b}"})
    assert response_b.json() == []
