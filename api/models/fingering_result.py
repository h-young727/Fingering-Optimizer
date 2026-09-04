import uuid
from datetime import datetime
from typing import TYPE_CHECKING

from sqlalchemy import ForeignKey, String, Text, UniqueConstraint
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy.sql import func

from api.dependencies.database import Base

# Avoids circular import error
if TYPE_CHECKING:
    from api.models.piece import Piece
    from api.models.weight_preset import WeightPreset


class FingeringResult(Base):
    __tablename__ = "fingering_results"
    __table_args__ = (UniqueConstraint("run_id", "track_index", name="uq_fingering_result_run_track"),)

    id: Mapped[int] = mapped_column(primary_key=True)
    piece_id: Mapped[int] = mapped_column(ForeignKey("pieces.id", ondelete="CASCADE"), nullable=False, index=True)

    # Enables results to persist through preset deletion
    weight_preset_id: Mapped[int | None] = mapped_column(ForeignKey("weight_presets.id", ondelete="SET NULL"), index=True)

    # Groups the results of each track produced by a single optimize call
    run_id: Mapped[str] = mapped_column(String(36), default=lambda: str(uuid.uuid4()), index=True)

    track_index: Mapped[int] = mapped_column()
    tab_text: Mapped[str] = mapped_column(Text)

    # Captures weights used at generation time rather than current preset values
    string_weight: Mapped[float] = mapped_column()
    fret_stretch: Mapped[float] = mapped_column()
    transition: Mapped[float] = mapped_column()
    fret_position: Mapped[float] = mapped_column()
    string_skip: Mapped[float] = mapped_column()

    created_at: Mapped[datetime] = mapped_column(server_default=func.now())

    piece: Mapped["Piece"] = relationship(back_populates="fingering_results")
    weight_preset: Mapped["WeightPreset | None"] = relationship(back_populates="fingering_results")
