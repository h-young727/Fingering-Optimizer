from fastapi import HTTPException, status
from sqlalchemy.exc import IntegrityError
from sqlalchemy.orm import Session

from api.models import User, WeightPreset
from api.schemas.weight_preset import WeightPresetCreate


def create_preset(db: Session, user: User, preset_in: WeightPresetCreate) -> WeightPreset:
    preset = WeightPreset(
        user_id=user.id,
        name=preset_in.name,
        string_weight=preset_in.string_weight,
        fret_stretch=preset_in.fret_stretch,
        transition=preset_in.transition,
        fret_position=preset_in.fret_position,
        string_skip=preset_in.string_skip,
    )
    db.add(preset)

    try:
        db.commit()
    except IntegrityError:
        db.rollback()
        raise HTTPException(status_code=status.HTTP_409_CONFLICT, detail="A preset with this name already exists") from None

    db.refresh(preset)

    return preset


def list_presets(db: Session, user: User) -> list[WeightPreset]:
    return db.query(WeightPreset).filter(WeightPreset.user_id == user.id).order_by(WeightPreset.name).all()


def get_owned_preset(db: Session, user: User, preset_id: int) -> WeightPreset:
    preset = db.get(WeightPreset, preset_id)

    if preset is None or preset.user_id != user.id:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Weight preset not found")

    return preset


def delete_preset(db: Session, user: User, preset_id: int) -> None:
    preset = get_owned_preset(db, user, preset_id)
    db.delete(preset)
    db.commit()
