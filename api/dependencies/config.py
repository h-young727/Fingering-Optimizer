from pydantic_settings import BaseSettings, SettingsConfigDict


class Settings(BaseSettings):
    model_config = SettingsConfigDict(env_file=".env", env_prefix="FO_")

    app_host: str = "127.0.0.1"
    app_port: int = 8000
    database_url: str = "sqlite:///./fingering_optimizer.db"
    upload_dir: str = "uploads"

    jwt_secret_key: str = "wysiWYSIwysiWYSIwysiWYSIwysiWYSI"
    jwt_algorithm: str = "HS256"
    access_token_expire_minutes: int = 60 * 24


conf = Settings()
