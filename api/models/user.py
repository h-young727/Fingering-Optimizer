from datetime import datetime
from typing import TYPE_CHECKING

from sqlalchemy import String
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy.sql import func

from api.dependencies.database import Base

# Avoids circular import error
if TYPE_CHECKING:
    from api.models.piece import Piece
    from api.models.weight_preset import WeightPreset


class User(Base):
    __tablename__ = "users"

    id: Mapped[int] = mapped_column(primary_key=True)
    email: Mapped[str] = mapped_column(String(255), unique=True, nullable=False, index=True)
    hashed_password: Mapped[str] = mapped_column(String(255), nullable=False)
    created_at: Mapped[datetime] = mapped_column(server_default=func.now())

    pieces: Mapped[list["Piece"]] = relationship(back_populates="user", cascade="all, delete-orphan")
    weight_presets: Mapped[list["WeightPreset"]] = relationship(back_populates="user", cascade="all, delete-orphan")
