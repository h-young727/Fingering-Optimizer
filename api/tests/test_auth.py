def test_signup_creates_user(client):
    response = client.post("/auth/signup", json={"email": "new@example.com", "password": "correcthorse"})

    assert response.status_code == 201
    body = response.json()
    assert body["email"] == "new@example.com"
    assert "password" not in body
    assert "hashed_password" not in body


def test_signup_short_password_rejected(client):
    response = client.post("/auth/signup", json={"email": "shortpw@example.com", "password": "short"})

    assert response.status_code == 422


def test_signup_duplicate_email_rejected(client):
    client.post("/auth/signup", json={"email": "dup@example.com", "password": "correcthorse"})
    response = client.post("/auth/signup", json={"email": "dup@example.com", "password": "correcthorse"})

    assert response.status_code == 409


def test_login_wrong_password_rejected(client):
    client.post("/auth/signup", json={"email": "login@example.com", "password": "correcthorse"})
    response = client.post("/auth/login", json={"email": "login@example.com", "password": "wrong"})

    assert response.status_code == 401


def test_login_returns_usable_token(client):
    client.post("/auth/signup", json={"email": "login2@example.com", "password": "correcthorse"})
    login_response = client.post("/auth/login", json={"email": "login2@example.com", "password": "correcthorse"})

    assert login_response.status_code == 200
    token = login_response.json()["access_token"]

    me_response = client.get("/auth/me", headers={"Authorization": f"Bearer {token}"})
    assert me_response.status_code == 200
    assert me_response.json()["email"] == "login2@example.com"


def test_me_without_token_rejected(client):
    response = client.get("/auth/me")

    assert response.status_code == 401
