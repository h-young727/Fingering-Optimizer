from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from api.controllers import fingering_result as fingering_result_controller
from api.dependencies.auth import get_current_user
from api.dependencies.database import get_db
from api.models import FingeringResult, User
from api.schemas.fingering_result import FingeringResultRead, OptimizeRequest

router = APIRouter(prefix="/pieces", tags=["fingering-results"])


@router.post("/{piece_id}/optimize", response_model=list[FingeringResultRead])
def optimize_piece(
    piece_id: int,
    request: OptimizeRequest,
    db: Session = Depends(get_db),
    current_user: User = Depends(get_current_user),
) -> list[FingeringResult]:
    return fingering_result_controller.optimize_piece(db, current_user, piece_id, request.weight_preset_id, request.weights)


@router.get("/{piece_id}/results", response_model=list[FingeringResultRead])
def list_results(
    piece_id: int, db: Session = Depends(get_db), current_user: User = Depends(get_current_user)
) -> list[FingeringResult]:
    return fingering_result_controller.list_results(db, current_user, piece_id)
