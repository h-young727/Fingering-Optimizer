import shutil
import uuid
from pathlib import Path

from fastapi import HTTPException, UploadFile, status
from sqlalchemy.orm import Session

from api.dependencies.config import conf
from api.models import Piece, User

ALLOWED_EXTENSIONS = {".mid", ".midi"}


def create_piece(db: Session, user: User, upload_file: UploadFile) -> Piece:
    extension = Path(upload_file.filename).suffix.lower()

    if extension not in ALLOWED_EXTENSIONS:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="File must be a .mid or .midi file")

    upload_dir = Path(conf.upload_dir)
    upload_dir.mkdir(parents=True, exist_ok=True)

    stored_path = upload_dir / f"{uuid.uuid4()}{extension}"

    with stored_path.open("wb") as out_file:
        shutil.copyfileobj(upload_file.file, out_file)

    piece = Piece(user_id=user.id, filename=upload_file.filename, file_path=str(stored_path))
    db.add(piece)
    db.commit()
    db.refresh(piece)

    return piece


def list_pieces(db: Session, user: User) -> list[Piece]:
    return db.query(Piece).filter(Piece.user_id == user.id).order_by(Piece.uploaded_at.desc()).all()


def get_owned_piece(db: Session, user: User, piece_id: int) -> Piece:
    piece = db.get(Piece, piece_id)

    if piece is None or piece.user_id != user.id:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Piece not found")

    return piece
