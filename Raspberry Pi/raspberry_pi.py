import pyrebase
from firebase import firebase
from time import sleep
from pusher_push_notifications import PushNotifications
import forecastio
lat = ****
lng = ****
firebase = firebase.FirebaseApplication("****")
api_key = "****"
config = {
    'apiKey': "****",
    'authDomain': "****",
    'databaseURL': "****",
    'projectId': "****",
    'storageBucket': "****",
    'messagingSenderId': "****",
  }
firebasea = pyrebase.initialize_app(config)
db=firebasea.database()
pn_client = PushNotifications(
    instance_id='****',
    secret_key='****',
)
def stream_handler(message):
    print(message)
    if(message['data'] is 1):
        response =pn_client.publish(
            interests=['hello'],
            publish_body={
                'apns': {
                    'aps': {
                        'alert': 'Hello',
                    },
                },
                'fcm': {
                    'notification': {
                        'title': 'Alarm',
                        'body': 'Zaznamenaný pohyb',
                    },
                },
            },
        )

        print(response['publishId'])
my_stream = db.child("pohyb").stream(stream_handler,None)


def stream_handler(message):
    print(message)
    if(message['data'] is 1):
        response =pn_client.publish(
            interests=['hello'],
            publish_body={
                'apns': {
                    'aps': {
                        'alert': 'Hello',
                    },
                },
                'fcm': {
                    'notification': {
                        'title': 'Zavlažovanie',
                        'body': 'Spustené zavlažovanie',
                    },
                },
            },
        )

        print(response['publishId'])
my_stream = db.child("zapnute_zavlazovanie").stream(stream_handler,None)

while True:
    forecast = forecastio.load_forecast(api_key, lat, lng)
    byHour = forecast.hourly()
    data = []
    for hourlyData in byHour.data:
        data.append(hourlyData.precipProbability)
    data = data[:12]
    sumA = 0
    firebase.put('', 'oblacik', byHour.icon)
    firebase.put('', 'hpredpoved', byHour.summary)
    current =forecast.currently()
    firebase.put('', 'summary', current.summary)
    firebase.put('', 'temperature', current.temperature)
    firebase.put('', 'windSpeed', current.windSpeed)
    firebase.put('', 'pressure', current.pressure)
    firebase.put('', 'uvIndex', current.uvIndex)
    firebase.put('', 'ozone', current.ozone)

    for item in data:
        sumA += float(item)

    if len(data) > 0:
        OverallProb = str(round(sumA / (float(12)), 3)*100)
    else:
        OverallProb = 0
    temperature = 0
    firebase.put('', 'pravdepodobnost', OverallProb)
    sleep(1200)
