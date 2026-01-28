"""Pytest fixtures for integration tests."""

import os
import subprocess
import time
from pathlib import Path

import pytest

# Constants
HA_URL = "http://localhost:8123"
ESPHOME_API_PORT = 6053
PROJECT_ROOT = Path(__file__).parent.parent.parent
DEVICES_DIR = PROJECT_ROOT / "devices"


@pytest.fixture(scope="session")
def docker_compose_up():
    """Start Docker Compose services."""
    integration_dir = Path(__file__).parent

    # Start services
    subprocess.run(
        ["docker-compose", "up", "-d"],
        cwd=integration_dir,
        check=True,
    )

    # Wait for HA to be ready
    max_wait = 120
    start_time = time.time()
    while time.time() - start_time < max_wait:
        try:
            import requests
            response = requests.get(f"{HA_URL}/api/", timeout=5)
            if response.status_code in (200, 401):  # 401 means HA is up but needs auth
                break
        except Exception:
            pass
        time.sleep(5)

    yield

    # Cleanup
    subprocess.run(
        ["docker-compose", "down", "-v"],
        cwd=integration_dir,
        check=True,
    )


@pytest.fixture(scope="session")
def esphome_host_device(docker_compose_up):
    """Start ESPHome device in host mode."""
    device_config = DEVICES_DIR / "host_test_device.yaml"

    # Create a minimal secrets file for testing
    secrets_file = DEVICES_DIR / "secrets.yaml"
    if not secrets_file.exists():
        secrets_file.write_text(
            'api_encryption_key: "dGVzdC1lbmNyeXB0aW9uLWtleS0zMi1ieXRlcyEh"\n'
        )

    # Start ESPHome in host mode
    proc = subprocess.Popen(
        ["esphome", "run", str(device_config), "--no-logs", "--device", "host"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=PROJECT_ROOT,
    )

    # Wait for device to start
    time.sleep(10)

    yield proc

    # Cleanup
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()


@pytest.fixture
def ha_api_url():
    """Return the Home Assistant API URL."""
    return HA_URL


@pytest.fixture
def esphome_device_host():
    """Return the ESPHome device host (for manual connection)."""
    return "host.docker.internal"  # Works on Docker for Mac/Windows
