import requests
import time
import sys

# Configuration
ESP_IP = "192.168.4.1"  # Default AP IP. Change if connected to Router.
BASE_URL = f"http://{ESP_IP}"

def test_connection():
    """Check if ESP32 is reachable."""
    print(f"Testing connection to {BASE_URL}...")
    try:
        response = requests.get(BASE_URL, timeout=5)
        if response.status_code == 200:
            print("✅ Connection Successful: Web Interface reachable.")
            return True
        else:
            print(f"❌ Connection Failed: Status Code {response.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        print(f"❌ Connection Error: {e}")
        print("   -> Ensure you are connected to the 'SPYDER_ROBOT' WiFi network.")
        return False

def test_sensors():
    """Test /sensors endpoint."""
    url = f"{BASE_URL}/sensors"
    print(f"\nTesting Sensors API: {url}")
    try:
        response = requests.get(url, timeout=2)
        if response.status_code == 200:
            data = response.json()
            print(f"✅ Sensors Data Received: {data}")
            if "ultrasonic1" in data:
                print("   -> Ultrasonic key found.")
            else:
                print("   ❌ Missing 'ultrasonic1' key in JSON.")
        else:
            print(f"❌ Sensors API Failed: {response.status_code}")
    except Exception as e:
        print(f"❌ Sensors Test Error: {e}")

def test_commands():
    """Test movement commands."""
    commands = ["START_ROBOT", "STOP_ROBOT", "BACK_ROBOT"]
    print(f"\nTesting Commands...")
    
    for cmd in commands:
        url = f"{BASE_URL}/command?cmd={cmd}"
        try:
            response = requests.get(url, timeout=2)
            if response.status_code == 200:
                print(f"✅ Command '{cmd}' sent successfully.")
            else:
                print(f"❌ Command '{cmd}' failed: {response.status_code}")
            time.sleep(0.5) # Small delay between commands
        except Exception as e:
            print(f"❌ Command '{cmd}' Error: {e}")

def test_servos():
    """Test individual servo control."""
    # Test setting Servo 1 (Pin 13) to 90 degrees
    pin = 13
    angle = 90
    url = f"{BASE_URL}/servo/{pin}?position={angle}"
    print(f"\nTesting Servo Control: {url}")
    try:
        response = requests.get(url, timeout=2)
        if response.status_code == 200:
            print(f"✅ Servo {pin} set to {angle}°.")
        else:
            print(f"❌ Servo Request Failed: {response.status_code}")
    except Exception as e:
        print(f"❌ Servo Test Error: {e}")

if __name__ == "__main__":
    print("=== ESP32 Spider Robot API Tester ===")
    
    if test_connection():
        test_sensors()
        test_commands()
        test_servos()
        print("\n=== Testing Complete ===")
    else:
        print("\n❌ Aborting tests due to connection failure.")
        sys.exit(1)
