from fastapi import APIRouter, Depends, File, UploadFile
from sqlalchemy.orm import Session

from api.controllers import piece as piece_controller
from api.dependencies.auth import get_current_user
from api.dependencies.database import get_db
from api.models import Piece, User
from api.schemas.piece import PieceRead

router = APIRouter(prefix="/pieces", tags=["pieces"])


@router.post("", response_model=PieceRead, status_code=201)
def upload_piece(
    file: UploadFile = File(...), db: Session = Depends(get_db), current_user: User = Depends(get_current_user)
) -> Piece:
    return piece_controller.create_piece(db, current_user, file)


@router.get("", response_model=list[PieceRead])
def list_pieces(db: Session = Depends(get_db), current_user: User = Depends(get_current_user)) -> list[Piece]:
    return piece_controller.list_pieces(db, current_user)
