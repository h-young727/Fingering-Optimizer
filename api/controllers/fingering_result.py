import uuid

import fingering_optimizer as engine
from fastapi import HTTPException, status
from sqlalchemy.orm import Session

from api.controllers import piece as piece_controller
from api.models import FingeringResult, User, WeightPreset
from api.schemas.weight_preset import WeightsIn


def resolve_weights(db: Session, user: User, weight_preset_id: int | None, weights_in: WeightsIn | None) -> tuple[engine.Weights, int | None]:
    if weight_preset_id is not None:
        preset = db.get(WeightPreset, weight_preset_id)

        if preset is None or preset.user_id != user.id:
            raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Weight preset not found")

        weights = engine.Weights()
        weights.string_weight = preset.string_weight
        weights.fret_stretch = preset.fret_stretch
        weights.transition = preset.transition
        weights.fret_position = preset.fret_position
        weights.string_skip = preset.string_skip

        return weights, preset.id

    weights_in = weights_in or WeightsIn()
    weights = engine.Weights()
    weights.string_weight = weights_in.string_weight
    weights.fret_stretch = weights_in.fret_stretch
    weights.transition = weights_in.transition
    weights.fret_position = weights_in.fret_position
    weights.string_skip = weights_in.string_skip

    return weights, None


def optimize_piece(
    db: Session, user: User, piece_id: int, weight_preset_id: int | None, weights_in: WeightsIn | None
) -> list[FingeringResult]:
    piece = piece_controller.get_owned_piece(db, user, piece_id)
    weights, resolved_preset_id = resolve_weights(db, user, weight_preset_id, weights_in)

    try:
        track_results = engine.optimize_midi_file(piece.file_path, weights)
    except RuntimeError as e:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=str(e)) from e

    run_id = str(uuid.uuid4())
    results = []

    for track_result in track_results:
        result = FingeringResult(
            piece_id=piece.id,
            weight_preset_id=resolved_preset_id,
            run_id=run_id,
            track_index=track_result.track_index,
            tab_text=track_result.tab,
            string_weight=weights.string_weight,
            fret_stretch=weights.fret_stretch,
            transition=weights.transition,
            fret_position=weights.fret_position,
            string_skip=weights.string_skip,
        )
        db.add(result)
        results.append(result)

    db.commit()

    for result in results:
        db.refresh(result)

    return results


def list_results(db: Session, user: User, piece_id: int) -> list[FingeringResult]:
    piece = piece_controller.get_owned_piece(db, user, piece_id)
    return (
        db.query(FingeringResult)
        .filter(FingeringResult.piece_id == piece.id)
        .order_by(FingeringResult.created_at.desc())
        .all()
    )
