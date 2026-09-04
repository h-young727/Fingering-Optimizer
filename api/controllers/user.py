from fastapi import HTTPException, status
from sqlalchemy.orm import Session

from api.dependencies.auth import create_access_token, hash_password, verify_password
from api.models import User
from api.schemas.user import UserCreate


def signup(db: Session, user_in: UserCreate) -> User:
    existing = db.query(User).filter(User.email == user_in.email).first()

    if existing is not None:
        raise HTTPException(status_code=status.HTTP_409_CONFLICT, detail="Email already registered")

    user = User(email=user_in.email, hashed_password=hash_password(user_in.password))
    db.add(user)
    db.commit()
    db.refresh(user)

    return user


def login(db: Session, email: str, password: str) -> str:
    credentials_error = HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail="Incorrect email or password")

    user = db.query(User).filter(User.email == email).first()

    if user is None or not verify_password(password, user.hashed_password):
        raise credentials_error

    return create_access_token(user.id)
