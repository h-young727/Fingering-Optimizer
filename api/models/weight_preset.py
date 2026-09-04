from datetime import datetime
from typing import TYPE_CHECKING

from sqlalchemy import ForeignKey, String, UniqueConstraint
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy.sql import func

from api.dependencies.database import Base

# Avoids circular import error
if TYPE_CHECKING:
    from api.models.fingering_result import FingeringResult
    from api.models.user import User


class WeightPreset(Base):
    __tablename__ = "weight_presets"
    __table_args__ = (UniqueConstraint("user_id", "name", name="uq_weight_preset_user_name"),)

    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id", ondelete="CASCADE"), nullable=False, index=True)
    name: Mapped[str] = mapped_column(String(100), nullable=False)
    string_weight: Mapped[float] = mapped_column(default=1.0)
    fret_stretch: Mapped[float] = mapped_column(default=1.0)
    transition: Mapped[float] = mapped_column(default=1.0)
    fret_position: Mapped[float] = mapped_column(default=1.0)
    string_skip: Mapped[float] = mapped_column(default=1.0)
    created_at: Mapped[datetime] = mapped_column(server_default=func.now())

    user: Mapped["User"] = relationship(back_populates="weight_presets")
    fingering_results: Mapped[list["FingeringResult"]] = relationship(back_populates="weight_preset")
