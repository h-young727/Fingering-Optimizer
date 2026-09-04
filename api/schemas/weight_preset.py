from datetime import datetime

from pydantic import BaseModel, ConfigDict


class WeightsIn(BaseModel):
    string_weight: float = 1.0
    fret_stretch: float = 1.0
    transition: float = 1.0
    fret_position: float = 1.0
    string_skip: float = 1.0


class WeightPresetCreate(BaseModel):
    name: str
    string_weight: float = 1.0
    fret_stretch: float = 1.0
    transition: float = 1.0
    fret_position: float = 1.0
    string_skip: float = 1.0


class WeightPresetRead(BaseModel):
    model_config = ConfigDict(from_attributes=True)

    id: int
    name: str
    string_weight: float
    fret_stretch: float
    transition: float
    fret_position: float
    string_skip: float
    created_at: datetime
