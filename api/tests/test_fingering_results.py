import pytest

from api.tests.conftest import SAMPLE_MIDI


@pytest.fixture()
def uploaded_piece_id(auth_client):
    with SAMPLE_MIDI.open("rb") as f:
        response = auth_client.post("/pieces", files={"file": ("song.mid", f, "audio/midi")})

    return response.json()["id"]


def test_optimize_with_adhoc_weights(auth_client, uploaded_piece_id):
    response = auth_client.post(
        f"/pieces/{uploaded_piece_id}/optimize",
        json={"weights": {"string_weight": 2.0, "fret_stretch": 0.5, "transition": 0.5}},
    )

    assert response.status_code == 200
    results = response.json()
    assert len(results) > 0

    run_ids = {r["run_id"] for r in results}
    assert len(run_ids) == 1

    for r in results:
        assert r["weight_preset_id"] is None
        assert r["string_weight"] == 2.0


def test_optimize_with_preset(auth_client, uploaded_piece_id):
    preset_id = auth_client.post(
        "/presets", json={"name": "My Preset", "string_weight": 3.0, "fret_stretch": 0.2, "transition": 0.2}
    ).json()["id"]

    response = auth_client.post(f"/pieces/{uploaded_piece_id}/optimize", json={"weight_preset_id": preset_id})

    assert response.status_code == 200
    results = response.json()
    for r in results:
        assert r["weight_preset_id"] == preset_id
        assert r["string_weight"] == 3.0


def test_optimize_with_nonexistent_preset_rejected(auth_client, uploaded_piece_id):
    response = auth_client.post(f"/pieces/{uploaded_piece_id}/optimize", json={"weight_preset_id": 99999})

    assert response.status_code == 404


def test_optimize_nonexistent_piece_rejected(auth_client):
    response = auth_client.post("/pieces/99999/optimize", json={})

    assert response.status_code == 404


def test_optimize_unparseable_midi_rejected(auth_client, tmp_path):
    garbage_file = tmp_path / "garbage.mid"
    garbage_file.write_bytes(b"this is not a real midi file")

    with garbage_file.open("rb") as f:
        piece_id = auth_client.post("/pieces", files={"file": ("garbage.mid", f, "audio/midi")}).json()["id"]

    response = auth_client.post(f"/pieces/{piece_id}/optimize", json={})

    assert response.status_code == 400


def test_list_results(auth_client, uploaded_piece_id):
    auth_client.post(f"/pieces/{uploaded_piece_id}/optimize", json={})

    response = auth_client.get(f"/pieces/{uploaded_piece_id}/results")

    assert response.status_code == 200
    assert len(response.json()) > 0


def test_list_results_nonexistent_piece_rejected(auth_client):
    response = auth_client.get("/pieces/99999/results")

    assert response.status_code == 404


def test_deleting_preset_nulls_reference_but_keeps_snapshot(auth_client, uploaded_piece_id):
    preset_id = auth_client.post("/presets", json={"name": "Doomed Preset", "string_weight": 4.0}).json()["id"]
    auth_client.post(f"/pieces/{uploaded_piece_id}/optimize", json={"weight_preset_id": preset_id})

    auth_client.delete(f"/presets/{preset_id}")

    results = auth_client.get(f"/pieces/{uploaded_piece_id}/results").json()
    assert len(results) > 0

    for r in results:
        assert r["weight_preset_id"] is None
        assert r["string_weight"] == 4.0
