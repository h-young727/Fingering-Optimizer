from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from api.controllers import weight_preset as weight_preset_controller
from api.dependencies.auth import get_current_user
from api.dependencies.database import get_db
from api.models import User, WeightPreset
from api.schemas.weight_preset import WeightPresetCreate, WeightPresetRead

router = APIRouter(prefix="/presets", tags=["presets"])


@router.post("", response_model=WeightPresetRead, status_code=201)
def create_preset(
    preset_in: WeightPresetCreate, db: Session = Depends(get_db), current_user: User = Depends(get_current_user)
) -> WeightPreset:
    return weight_preset_controller.create_preset(db, current_user, preset_in)


@router.get("", response_model=list[WeightPresetRead])
def list_presets(db: Session = Depends(get_db), current_user: User = Depends(get_current_user)) -> list[WeightPreset]:
    return weight_preset_controller.list_presets(db, current_user)


@router.delete("/{preset_id}", status_code=204)
def delete_preset(preset_id: int, db: Session = Depends(get_db), current_user: User = Depends(get_current_user)) -> None:
    weight_preset_controller.delete_preset(db, current_user, preset_id)
