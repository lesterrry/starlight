class EthernetService {
  public:
    bool begin() {
      if (Ethernet.begin(_macAddress) == 0) return false;

      _server.begin();

      return true;
    }

    EthernetClient available() {
      return _server.available();
    }

    String localIP() {
      IPAddress ip = Ethernet.localIP();
      String ipString = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);

      return ipString;
    }

    String parseRequestRoute(String requestHeader) {
      int firstSpace = requestHeader.indexOf(' ');
      int secondSpace = requestHeader.indexOf(' ', firstSpace + 1);

      String route;
      if (firstSpace != -1) {
        if (secondSpace != -1) {
          route = requestHeader.substring(firstSpace + 1, secondSpace);
        } else {
          route = requestHeader.substring(firstSpace + 1);
        }
      }
      
      return route;
    }

    void respond(EthernetClient client, String response) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println(response);
    }

  private:
    EthernetServer _server = EthernetServer(80);
    byte _macAddress[6] = MAC_ADDRESS;
};
