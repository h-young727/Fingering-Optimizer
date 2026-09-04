from datetime import datetime
from typing import TYPE_CHECKING

from sqlalchemy import ForeignKey, String
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy.sql import func

from api.dependencies.database import Base

# Avoids circular import error
if TYPE_CHECKING:
    from api.models.fingering_result import FingeringResult
    from api.models.user import User


class Piece(Base):
    __tablename__ = "pieces"

    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), nullable=False, index=True)
    filename: Mapped[str] = mapped_column(String(255), nullable=False)
    file_path: Mapped[str] = mapped_column(String(512), nullable=False)
    uploaded_at: Mapped[datetime] = mapped_column(server_default=func.now())

    user: Mapped["User"] = relationship(back_populates="pieces")
    fingering_results: Mapped[list["FingeringResult"]] = relationship(back_populates="piece", cascade="all, delete-orphan")
