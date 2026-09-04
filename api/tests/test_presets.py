def test_create_and_list_preset(auth_client):
    create_response = auth_client.post(
        "/presets", json={"name": "Easy Mode", "string_weight": 2.0, "fret_stretch": 0.5, "transition": 0.5}
    )

    assert create_response.status_code == 201
    assert create_response.json()["name"] == "Easy Mode"

    list_response = auth_client.get("/presets")
    assert list_response.status_code == 200
    assert len(list_response.json()) == 1


def test_duplicate_preset_name_rejected(auth_client):
    auth_client.post("/presets", json={"name": "Easy Mode"})
    response = auth_client.post("/presets", json={"name": "Easy Mode"})

    assert response.status_code == 409


def test_delete_preset(auth_client):
    preset_id = auth_client.post("/presets", json={"name": "Temp Preset"}).json()["id"]

    delete_response = auth_client.delete(f"/presets/{preset_id}")
    assert delete_response.status_code == 204

    list_response = auth_client.get("/presets")
    assert list_response.json() == []


def test_delete_nonexistent_preset_rejected(auth_client):
    response = auth_client.delete("/presets/99999")

    assert response.status_code == 404
