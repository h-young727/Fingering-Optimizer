import uvicorn
from fastapi import FastAPI

from api.dependencies.config import conf
from api.dependencies.database import Base, engine
from api.routers import fingering_result, piece, user, weight_preset

Base.metadata.create_all(bind=engine)

app = FastAPI(title="Fingering Optimizer API")

app.include_router(fingering_result.router)
app.include_router(piece.router)
app.include_router(user.router)
app.include_router(weight_preset.router)


@app.get("/health")
def health_check() -> dict[str, str]:
    return {"status": "ok"}


if __name__ == "__main__":
    uvicorn.run(app, host=conf.app_host, port=conf.app_port)
