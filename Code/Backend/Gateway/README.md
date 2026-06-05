
### How to deploy 

1. Install following dependencies:
    - `sudo apt-get install bluez`
    - `sudo apt-get install dbus`
    - `sudo apt-get install bluetooth bluez-tools rfkill`
1. Copy `./Backend` to to the target
2. Run `sudo docker buildx build --platform linux/arm64 -t gateway --load -f ./Gateway/Dockerfile .`
3. Run `sudo docker save -o gateway.tar gateway`
3. Copy ble_receiver.tar to Raspberry Pi
4. Run `sudo docker load -i gateway.tar` on the Rapsberry Pi

### Run Gateway on a Raspberry PI

**Execute following command:**

```bash
sudo docker run --rm -it --privileged -p 5001:5001 -p 7093:7093 --net=host --env DBUS_SYSTEM_BUS_ADDRESS=unix:path=/run/dbus/system_bus_socket --env Network__Interface=eth0 --env Backend__IpAddress=https://{Backend-IpAddress}:7093 --env Keycloak__EndpointUrl=http://{AuthenticationProvider-IpAddress}:{Port} --env Keycloak__ClientId=smart-bin --env Keycloak__ClientSecret={KeyCloak-ClientSecret} -v /run/dbus/system_bus_socket:/run/dbus/system_bus_socket gateway
```

#### Env Variabels

**Backend-IpAddress:** Ip address or dns name of the backend.

**AuthenticationProvider-IpAddress:** Ip address or dns name of the KeyCloak instance.

**KeyCloak-ClientSecret:** Client secret to fetch the token.  