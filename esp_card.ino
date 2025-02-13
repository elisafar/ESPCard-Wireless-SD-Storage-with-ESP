/*
  Ali Safarli made with AI
*/
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Define the webserver
ESP8266WebServer server(80);

// SD card CS pin
#define SD_CS_PIN D8

// Define the Access Point credentials
const char* apSSID = "Espcard";      // Wi-Fi network name
const char* apPassword = "12345678";   // Wi-Fi password (must be at least 8 characters)

void handleRoot() {
  server.send(200, "text/html",
              "<html>"
              "<head>"
              "<style>"
              "body { display: flex; justify-content: center; align-items: center; height: 100vh; flex-direction: column; }"
              "input[type='file'] { margin: 10px; }"
              "#progress { width: 100%; background-color: #f3f3f3; }"
              "#bar { width: 0%; height: 30px; background-color: #4caf50; }"
              "</style>"
              "<script>"
              "function uploadFile() {"
              "  var fileInput = document.getElementById('fileInput');"
              "  var file = fileInput.files[0];"
              "  var formData = new FormData();"
              "  formData.append('data', file);"
              "  var xhr = new XMLHttpRequest();"
              "  xhr.open('POST', '/upload', true);"
              "  xhr.upload.onprogress = function(e) {"
              "    if (e.lengthComputable) {"
              "      var percentComplete = (e.loaded / e.total) * 100;"
              "      document.getElementById('bar').style.width = percentComplete + '%';"
              "    }"
              "  };"
              "  xhr.onload = function() {"
              "    if (xhr.status === 200) {"
              "      alert('File uploaded successfully!');"
              "    } else {"
              "      alert('Upload failed.');"
              "    }"
              "  };"
              "  xhr.send(formData);"
              "}"
              "</script>"
              "</head>"
              "<body>"
              "<form onsubmit='event.preventDefault(); uploadFile();'>"
              "<input type='file' id='fileInput' name='data' required><br>"
              "<input type='submit' value='Upload'>"
              "</form>"
              "<div id='progress'><div id='bar'></div></div>"
              "</body>"
              "</html>");
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/" + upload.filename;
    Serial.print("File Upload Start: ");
    Serial.println(filename);

    if (SD.exists(filename)) {
      SD.remove(filename); // Remove the file if it already exists
    }

    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file for writing");
      return;
    }
    file.close();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    String filename = "/" + upload.filename;
    File file = SD.open(filename, FILE_WRITE); // Use FILE_WRITE for appending
    if (file) {
      file.write(upload.buf, upload.currentSize);
      file.close();
    } else {
      Serial.println("Failed to open file for appending");
    }
    Serial.print("File Upload Write: ");
    Serial.println(upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.print("File Upload End. Total Size: ");
    Serial.println(upload.totalSize);
    server.send(200, "text/plain", "File uploaded successfully");
  } else {
    Serial.println("File upload failed");
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void setup() {
  Serial.begin(115200);

  // Start the ESP8266 in Access Point mode
  WiFi.softAP(apSSID, apPassword);
  IPAddress IP = WiFi.softAPIP(); // Get the AP IP address
  Serial.print("Access Point started. IP Address: ");
  Serial.println(IP);

  // Initialize the SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // // Set up web server routes
  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, []() {}, handleFileUpload);
  server.onNotFound(handleNotFound);

  // Start the server
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
} 
