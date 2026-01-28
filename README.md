# home-esp

ESPHome external components for Home Assistant with testable C++ architecture.

## Features

- **Testable Architecture**: Pure C++ business logic separated from ESPHome bindings
- **Unit Tests**: GoogleTest on native platform (fast, no hardware needed)
- **Integration Tests**: Docker-based Home Assistant testing
- **ESPHome Host Platform**: Run components on your desktop for development

## Project Structure

```
home-esp/
├── components/           # ESPHome external components
│   ├── example_sensor/   # Temperature sensor example
│   ├── example_actuator/ # Relay/switch example
│   └── example_bridge/   # RF433 protocol bridge example
├── lib/core/             # Testable C++ libraries
│   ├── interfaces/       # Pure virtual interfaces
│   └── adapters/         # ESPHome adapters
├── mocks/                # ESPHome API mocks for testing
├── test/
│   ├── native/           # GoogleTest unit tests
│   └── integration/      # Docker + pytest integration tests
└── devices/              # ESPHome device configs
```

## Quick Start

### Prerequisites

- Python 3.10+
- PlatformIO Core
- ESPHome
- Docker (for integration tests)

### Install Dependencies

```bash
pip install platformio esphome pytest requests
```

### Run Unit Tests

```bash
pio test -e native
```

### Validate ESPHome Configs

```bash
# Copy secrets template
cp devices/secrets.yaml.example devices/secrets.yaml
# Edit with your values

# Validate
esphome config devices/esp32_dev.yaml
```

### Run on Host Platform (Desktop)

```bash
esphome run devices/host_test_device.yaml
```

Then add to Home Assistant manually using your computer's IP address.

### Run Integration Tests

```bash
cd test/integration
docker-compose up -d
pytest
docker-compose down
```

## Architecture

The project uses a hybrid testing approach:

```
┌─────────────────────────────────────────────────────────────┐
│                    Business Logic (Pure C++)                 │
│              TemperatureReader, RelayController, etc.        │
├─────────────────────────────────────────────────────────────┤
│                    Interfaces (Pure Virtual)                 │
│         ISensorPublisher, ICommandHandler, IProtocolCodec   │
└─────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴─────────┐
                    ▼                   ▼
          ┌─────────────────┐  ┌─────────────────┐
          │  ESPHome Adapter │  │  Mock (tests)   │
          └─────────────────┘  └─────────────────┘
```

- **Business logic** is pure C++ with no ESPHome dependencies
- **Interfaces** define contracts between logic and platform
- **Adapters** bridge interfaces to ESPHome classes
- **Mocks** implement interfaces for fast unit testing

## Creating a New Component

1. Create business logic in `lib/core/`:

```cpp
// lib/core/my_logic.h
class MyLogic {
 public:
  explicit MyLogic(ISensorPublisher* publisher);
  void process_data(int raw_value);
};
```

2. Create tests in `test/native/`:

```cpp
// test/native/test_my_logic.cpp
TEST(MyLogicTest, ProcessesDataCorrectly) {
  MockSensorPublisher publisher;
  MyLogic logic(&publisher);
  logic.process_data(42);
  EXPECT_EQ(publisher.get_publish_count(), 1);
}
```

3. Create ESPHome component in `components/`:

```
components/my_component/
├── __init__.py      # Config schema
├── sensor.py        # Platform registration
└── my_component.h   # C++ component
```

4. Add to device YAML:

```yaml
external_components:
  - source: {type: local, path: ../components}

my_component:
  id: my_thing
```

## CI Pipeline

GitHub Actions runs:
1. **Unit Tests** - GoogleTest on native platform
2. **ESPHome Validation** - Config syntax check
3. **ESPHome Compile** - Full ESP32 firmware build
4. **Lint** - Python (Black, flake8) + YAML
5. **Integration Tests** - Docker Home Assistant + pytest

## License

MIT
