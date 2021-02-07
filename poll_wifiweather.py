#!/usr/bin/python3
import sys
from time import sleep
import requests

url = 'http://192.168.0.197/json'

# https://www.domoticz.com/wiki/Domoticz_API/JSON_URL's#Create_a_virtual_sensor

if __name__ == '__main__':
    while True:
        try:
            resp = requests.get(url=url)
            data = resp.json()
            print(data)

            inside = data["DS_INS"]
            outside = data["DS_OUT"]

            bme_t = data["BME_T"]
            bme_hum = data["BME_HUM"]
            bme_pres = data["BME_PRES"]
            head_idx = data["HEAT_IDX"]

            set_inside_url = "http://192.168.0.149:9002/json.htm?type=command&param=udevice&idx=6&nvalue=0&svalue=" + str(inside)
            requests.get(url=set_inside_url)

            set_outside_url = "http://192.168.0.149:9002/json.htm?type=command&param=udevice&idx=7&nvalue=0&svalue=" + str(outside)
            requests.get(url=set_outside_url)

            set_bme_url = "http://192.168.0.149:9002/json.htm?type=command&param=udevice&idx=10&nvalue=0&svalue=" + str(bme_t) + ";" + str(bme_hum) + ";1;" + str(bme_pres / 100) + ";1"
            requests.get(url=set_bme_url)

            sleep(5)
        except requests.exceptions.HTTPError as errh:
            print("Http Error:", errh)
        except requests.exceptions.ConnectionError as errc:
            print("Error Connecting:", errc)
        except requests.exceptions.Timeout as errt:
            print("Timeout Error:", errt)
        except requests.exceptions.RequestException as err:
            print("OOps: Something Else", err)


