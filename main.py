from fastapi import FastAPI, HTTPException
import pyrebase
from dotenv import load_dotenv
import os

# Load environment variables
load_dotenv()

# Set up app
app = FastAPI()

# Retrieve Firebase configuration from environment variables
firebase_config: dict[str, str] = {
    "apiKey": os.getenv("APIKEY"),
    "authDomain": os.getenv("AUTHDOMAIN"),
    "databaseURL": os.getenv("DATABASEURL"),
    "storageBucket": os.getenv("STORAGEBUCKET"),
}

# Initialize Firebase
firebase = pyrebase.initialize_app(firebase_config)
db = firebase.database()

@app.get("/confirm_room")
async def confirm_room(card: str):
    # Retrieve the allowed rooms
    allowed_rooms = db.child("allowedRooms").get().val()

    # Check if the card is allowed for any room
    for room, cards in allowed_rooms.items():
        if card in cards:
            return {"room": room, "access": True}

    return {"room": None, "access": False}

@app.post("/card_data")
async def receive_card_data(card_id: str):
    try:
        # Check if the card is associated with a room
        rooms = db.child("allowedRooms").get().val()
        assigned_room = None

        for room, cards in rooms.items():
            if card_id in cards:
                assigned_room = room
                break

        if assigned_room is None:
            return {"status": "error", "message": "Card not associated with any room"}

        # Store the card data in Firebase
        data = {
            "card_id": card_id,
            "room_id": assigned_room,
            "timestamp": {".sv": "timestamp"}  # Server-side timestamp
        }
        db.child("card_readings").push(data)
        
        return {"status": "success", "message": f"Card data stored successfully for room {assigned_room}"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/")
async def root():
    return {"message": "Card Data and Room Confirmation API"}
