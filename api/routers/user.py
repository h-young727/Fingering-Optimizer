from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from api.controllers import user as user_controller
from api.dependencies.auth import get_current_user
from api.dependencies.database import get_db
from api.models import User
from api.schemas.user import Token, UserCreate, UserLogin, UserRead

router = APIRouter(prefix="/auth", tags=["auth"])


@router.post("/signup", response_model=UserRead, status_code=201)
def signup(user_in: UserCreate, db: Session = Depends(get_db)) -> User:
    return user_controller.signup(db, user_in)


@router.post("/login", response_model=Token)
def login(credentials: UserLogin, db: Session = Depends(get_db)) -> Token:
    access_token = user_controller.login(db, credentials.email, credentials.password)
    return Token(access_token=access_token)


@router.get("/me", response_model=UserRead)
def read_current_user(current_user: User = Depends(get_current_user)) -> User:
    return current_user
