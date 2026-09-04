from datetime import datetime

from pydantic import BaseModel, ConfigDict

from api.schemas.weight_preset import WeightsIn


class OptimizeRequest(BaseModel):
    weight_preset_id: int | None = None
    weights: WeightsIn | None = None


class FingeringResultRead(BaseModel):
    model_config = ConfigDict(from_attributes=True)

    id: int
    track_index: int
    tab_text: str
    run_id: str
    weight_preset_id: int | None
    string_weight: float
    fret_stretch: float
    transition: float
    fret_position: float
    string_skip: float
    created_at: datetime
