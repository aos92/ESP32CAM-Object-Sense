/*
 * ESP32-CAM Pelacakan Orang (tfjs coco-ssd)
 *
 * Servo1 (Rotasi Horizontal) -> gpio2 (Servo Motor dengan Ground Eksternal Bersama ESP32-CAM)
 * Servo2 (Rotasi Vertikal) -> gpio13 (Servo Motor dengan Ground Eksternal Bersama ESP32-CAM)
 * 
 * Kelas Objek
https://github.com/tensorflow/tfjs-models/blob/master/coco-ssd/src/classes.ts

 * Beranda
 * http://APIP
 * http://STAIP
 * 
 * Format Perintah Kustom:
 * http://APIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
 * http://STAIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
 * 
 * IP AP Default: 192.168.4.1
 * http://192.168.xxx.xxx?ip
 * http://192.168.xxx.xxx?mac
 * http://192.168.xxx.xxx?restart
 * http://192.168.xxx.xxx?flash=value //value= 0~255 Lampu Kilat
 * http://192.168.xxx.xxx?getstill //Dapatkan Gambar Video
 * http://192.168.xxx.xxx?framesize=size //size= UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA Ubah Resolusi Video
 * http://192.168.xxx.xxx?quality=value // nilai = 10 hingga 63
 * http://192.168.xxx.xxx?brightness=value // nilai = -2 hingga 2
 * http://192.168.xxx.xxx?contrast=value // nilai = -2 hingga 2
 * http://192.168.xxx.xxx?servo1=value //value= 1700~8000
 * http://192.168.xxx.xxx?servo2=value //value= 1700~8000
 * 
 * Query IP Klien:
 * Query IP: http://192.168.4.1/?ip
 * 
 * Reset Jaringan:  
 * http://192.168.4.1/?resetwifi=ssid;password
 * 
 */

// Mengatur koneksi WIFI dengan menyertakan nama dan kata sandi jaringan
const char* ssid     = "your_SSD";   // Nama SSID jaringan Anda
const char* password = "your_PASSWORD";   // Kata sandi jaringan Anda

int angle1Value1 = 4850; // Nilai sudut 1
int angle1Value2 = 4850; // Nilai sudut 2

// Mengatur koneksi ke titik akses (AP) dengan menyertakan nama dan kata sandi
const char* apssid = "your_AP_SSD"; // Nama SSID AP Anda
const char* appassword = "your_AP_PASSWORD";    // Kata sandi AP Anda, minimal delapan karakter

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"         // Library untuk kamera
#include "soc/soc.h"            // Digunakan untuk restart saat tegangan tidak stabil
#include "soc/rtc_cntl_reg.h"   // Digunakan untuk restart saat tegangan tidak stabil

String Feedback="";   // Variabel untuk menyimpan pesan balik kepada klien
// Variabel untuk menyimpan parameter instruksi
String Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
// Status parsing instruksi
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

// PERINGATAN!!! Pastikan Anda telah memilih Modul ESP32 Wrover,
//            atau papan lain yang memiliki PSRAM diaktifkan

// Konfigurasi pin untuk modul ESP32-CAM
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
 
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

WiFiServer server(80); // Membuat server HTTP pada port 80

void ExecuteCommand()
{
  //Serial.println("");
  //Serial.println("Command: "+Command);
  
  // Menampilkan log perintah ke serial monitor, kecuali perintah "getstill"
  if (cmd!="getstill") {
    Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
    Serial.println("");
  }
  
  // Memeriksa perintah yang diterima dan menjalankan aksi sesuai dengan perintah tersebut
  if (cmd=="your cmd") {
    // Anda dapat melakukan apapun.
    // Feedback="<font color=\"red\">Hello World</font>";
  }
  else if (cmd=="ip") {
    // Mengambil dan menampilkan alamat IP dari mode AP dan STA
    Feedback="AP IP: "+WiFi.softAPIP().toString();    
    Feedback+=", ";
    Feedback+="STA IP: "+WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    // Mengambil dan menampilkan alamat MAC dari STA
    Feedback="STA MAC: "+WiFi.macAddress();
  }  
  else if (cmd=="resetwifi") {  // Mengatur ulang koneksi WIFI
    // Mulai ulang koneksi WIFI dengan SSID dan password yang diberikan
    WiFi.begin(P1.c_str(), P2.c_str());
    Serial.print("Connecting to ");
    Serial.println(P1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    Feedback="STAIP: "+WiFi.localIP().toString();
  }    
  else if (cmd=="restart") {
    // Mulai ulang perangkat ESP32
    ESP.restart();
  }    
  else if (cmd=="flash") {
    // Mengatur intensitas cahaya LED
    ledcAttachPin(14, 4);  
    ledcSetup(4, 5000, 8);   
     
    int val = P1.toInt();
    ledcWrite(4,val);  
  }  
  else if (cmd=="framesize") { 
    // Mengatur ukuran frame kamera sesuai dengan nilai yang diberikan
    sensor_t * s = esp_camera_sensor_get();  
    if (P1=="QQVGA")
      s->set_framesize(s, FRAMESIZE_QQVGA);
    else if (P1=="HQVGA")
      s->set_framesize(s, FRAMESIZE_HQVGA);
    else if (P1=="QVGA")
      s->set_framesize(s, FRAMESIZE_QVGA);
    else if (P1=="CIF")
      s->set_framesize(s, FRAMESIZE_CIF);
    else if (P1=="VGA")
      s->set_framesize(s, FRAMESIZE_VGA);  
    else if (P1=="SVGA")
      s->set_framesize(s, FRAMESIZE_SVGA);
    else if (P1=="XGA")
      s->set_framesize(s, FRAMESIZE_XGA);
    else if (P1=="SXGA")
      s->set_framesize(s, FRAMESIZE_SXGA);
    else if (P1=="UXGA")
      s->set_framesize(s, FRAMESIZE_UXGA);           
    else 
      s->set_framesize(s, FRAMESIZE_QVGA);     
  }
  else if (cmd=="quality") { 
    // Mengatur kualitas gambar kamera sesuai dengan nilai yang diberikan
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_quality(s, val);
  }
  else if (cmd=="contrast") {
    // Mengatur kontras gambar kamera sesuai dengan nilai yang diberikan
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_contrast(s, val);
  }
  else if (cmd=="brightness") {
    // Mengatur kecerahan gambar kamera sesuai dengan nilai yang diberikan
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();  
    s->set_brightness(s, val);  
  }
  else if (cmd=="servo1") {
    // Mengatur posisi servo 1 sesuai dengan nilai yang diberikan
    int val = P1.toInt();
    ledcAttachPin(12, 3);  
    ledcSetup(3, 50, 16);      
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    ledcWrite(3, val); 
    Serial.println("servo1="+String(val));
  }  
  else if (cmd=="servo2") {
    // Mengatur posisi servo 2 sesuai dengan nilai yang diberikan
    int val = P1.toInt();
    ledcAttachPin(13, 5);  
    ledcSetup(5, 50, 16);      
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    ledcWrite(5, val); 
    Serial.println("servo2="+String(val));
  }   
  else {
    // Menampilkan pesan kesalahan jika perintah tidak dikenali
    Feedback="Perintah tidak ditentukan.";
  }
  
  // Menetapkan nilai Feedback jika tidak ada nilai yang dihasilkan sebelumnya
  if (Feedback=="") Feedback=Command;  
}

// Fungsi untuk memecah string perintah dan menyimpannya ke dalam variabel
void getCommand(char c)
{
  // Menetapkan status menerima karakter
  if (c=='?') ReceiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
  
  // Jika dalam proses menerima karakter
  if (ReceiveState==1)
  {
    // Menambahkan karakter ke string perintah dan variabel lainnya
    Command=Command+String(c);
    
    // Menetapkan status pemrosesan karakter perintah
    if (c=='=') cmdState=0;
    if (c==';') strState++;
  
    // Memisahkan string perintah dan parameter
    if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) P1=P1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) P2=P2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) P3=P3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) P4=P4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) P5=P5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) P6=P6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) P7=P7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) P8=P8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) P9=P9+String(c);
    
    // Menetapkan status karakter khusus
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
  }
}

// Mendefinisikan halaman dari antarmuka pengelolaan web yang disesuaikan.
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
 <!DOCTYPE html>
  <head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script>
  <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow/tfjs@1.3.1/dist/tf.min.js"> </script>
  <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow-models/coco-ssd@2.1.0"> </script>   
  </head><body>
  <img id="ShowImage" src="" style="display:none">
  <canvas id="canvas" width="0" height="0"></canvas>  
  <table>
  <tr>
    <td><input type="button" id="restart" value="Restart"></td> 
    <td colspan="2"><input type="button" id="getStill" value="Start Detect" style="display:none"></td> 
  </tr>
  <tr>  
    <td>Object</td> 
    <td colspan="2">
        <select id="object" onchange="count.innerHTML='';">
          <option value="person" selected="selected">person</option>
          <option value="bicycle">bicycle</option>
          <option value="car">car</option>
          <option value="motorcycle">motorcycle</option>
          <option value="airplane">airplane</option>
          <option value="bus">bus</option>
          <option value="train">train</option>
          <option value="truck">truck</option>
          <option value="boat">boat</option>
          <option value="traffic light">traffic light</option>
          <option value="fire hydrant">fire hydrant</option>
          <option value="stop sign">stop sign</option>
          <option value="parking meter">parking meter</option>
          <option value="bench">bench</option>
          <option value="bird">bird</option>
          <option value="cat">cat</option>
          <option value="dog">dog</option>
          <option value="horse">horse</option>
          <option value="sheep">sheep</option>
          <option value="cow">cow</option>
          <option value="elephant">elephant</option>
          <option value="bear">bear</option>
          <option value="zebra">zebra</option>
          <option value="giraffe">giraffe</option>
          <option value="backpack">backpack</option>
          <option value="umbrella">umbrella</option>
          <option value="handbag">handbag</option>
          <option value="tie">tie</option>
          <option value="suitcase">suitcase</option>
          <option value="frisbee">frisbee</option>
          <option value="skis">skis</option>
          <option value="snowboard">snowboard</option>
          <option value="sports ball">sports ball</option>
          <option value="kite">kite</option>
          <option value="baseball bat">baseball bat</option>
          <option value="baseball glove">baseball glove</option>
          <option value="skateboard">skateboard</option>
          <option value="surfboard">surfboard</option>
          <option value="tennis racket">tennis racket</option>
          <option value="bottle">bottle</option>
          <option value="wine glass">wine glass</option>
          <option value="cup">cup</option>
          <option value="fork">fork</option>
          <option value="knife">knife</option>
          <option value="spoon">spoon</option>
          <option value="bowl">bowl</option>
          <option value="banana">banana</option>
          <option value="apple">apple</option>
          <option value="sandwich">sandwich</option>
          <option value="orange">orange</option>
          <option value="broccoli">broccoli</option>
          <option value="carrot">carrot</option>
          <option value="hot dog">hot dog</option>
          <option value="pizza">pizza</option>
          <option value="donut">donut</option>
          <option value="cake">cake</option>
          <option value="chair">chair</option>
          <option value="couch">couch</option>
          <option value="potted plant">potted plant</option>
          <option value="bed">bed</option>
          <option value="dining table">dining table</option>
          <option value="toilet">toilet</option>
          <option value="tv">tv</option>
          <option value="laptop">laptop</option>
          <option value="mouse">mouse</option>
          <option value="remote">remote</option>
          <option value="keyboard">keyboard</option>
          <option value="cell phone">cell phone</option>
          <option value="microwave">microwave</option>
          <option value="oven">oven</option>
          <option value="toaster">toaster</option>
          <option value="sink">sink</option>
          <option value="refrigerator">refrigerator</option>
          <option value="book">book</option>
          <option value="clock">clock</option>
          <option value="vase">vase</option>
          <option value="scissors">scissors</option>
          <option value="teddy bear">teddy bear</option>
          <option value="hair drier">hair drier</option>
          <option value="toothbrush">toothbrush</option>
        </select>
    </td>
    <td><span id="count" style="color:red"><span></td>
  </tr>
  <tr>
    <td>ScoreLimit</td> 
    <td colspan="2">
      <select id="score">
        <option value="1.0">1</option>
        <option value="0.9">0.9</option>
        <option value="0.8">0.8</option>
        <option value="0.7">0.7</option>
        <option value="0.6">0.6</option>
        <option value="0.5">0.5</option>
        <option value="0.4">0.4</option>
        <option value="0.3">0.3</option>
        <option value="0.2">0.2</option>
        <option value="0.1">0.1</option>
        <option value="0" selected="selected">0</option>
      </select>
    </td>
  </tr>
  <tr>
    <td>MirrorImage</td> 
    <td colspan="2">  
      <select id="mirrorimage">
        <option value="1">yes</option>
        <option value="0">no</option>
      </select>
    </td>
  </tr>   
  <tr>
    <td>Resolution</td> 
    <td colspan="2">
      <select id="framesize">
        <option value="UXGA">UXGA(1600x1200)</option>
        <option value="SXGA">SXGA(1280x1024)</option>
        <option value="XGA">XGA(1024x768)</option>
        <option value="SVGA">SVGA(800x600)</option>
        <option value="VGA">VGA(640x480)</option>
        <option value="CIF">CIF(400x296)</option>
        <option value="QVGA" selected="selected">QVGA(320x240)</option>
        <option value="HQVGA">HQVGA(240x176)</option>
        <option value="QQVGA">QQVGA(160x120)</option>
      </select> 
    </td>
  </tr>    
  <tr>
    <td>Flash</td>
    <td colspan="2"><input type="range" id="flash" min="0" max="255" value="0"></td>
  </tr>
  <tr>
    <td>Quality</td>
    <td colspan="2"><input type="range" id="quality" min="10" max="63" value="10"></td>
  </tr>
  <tr>
    <td>Brightness</td>
    <td colspan="2"><input type="range" id="brightness" min="-2" max="2" value="0"></td>
  </tr>
  <tr>
    <td>Contrast</td>
    <td colspan="2"><input type="range" id="contrast" min="-2" max="2" value="0"></td>
  </tr>
  <tr>
    <td>Rotate</td>
    <td align="left" colspan="2">
      <select onchange="document.getElementById('canvas').style.transform='rotate('+this.value+')';">
        <option value="0deg">0deg</option>
        <option value="90deg">90deg</option>
        <option value="180deg">180deg</option>
        <option value="270deg">270deg</option>
      </select>
    </td>
  </tr>  
  </table>
  <div id="result" style="color:red"><div>
<script>
    // Mendapatkan elemen-elemen dari HTML
    var getStill = document.getElementById('getStill');
    var ShowImage = document.getElementById('ShowImage');
    var canvas = document.getElementById("canvas");
    var context = canvas.getContext("2d"); 
    var object = document.getElementById('object');
    var score = document.getElementById("score");
    var mirrorimage = document.getElementById("mirrorimage");    
    var count = document.getElementById('count');     
    var result = document.getElementById('result');
    var flash = document.getElementById('flash'); 
    var lastValue="";
    var myTimer;  
    var restartCount=0;     
    var Model;
    var angle1Value1 = 4850;
    var angle1Value2 = 4850;        

    // Event handler saat tombol "getStill" diklik
    getStill.onclick = function (event) { 
      clearInterval(myTimer);   
      // Set interval untuk memanggil fungsi error_handle setiap 5 detik
      myTimer = setInterval(function(){error_handle();},5000);
      // Mengatur atribut src dari elemen ShowImage dengan URL yang berbeda setiap kali tombol diklik
      ShowImage.src=location.origin+'/?getstill='+Math.random();
    }

    // Fungsi untuk menangani kesalahan saat mendapatkan gambar
    function error_handle() {
      restartCount++;
      clearInterval(myTimer);
      if (restartCount<=2) {
        // Menampilkan pesan error dan mengatur interval untuk memanggil fungsi getStill setiap 10 detik
        result.innerHTML = "Get still error. <br>Restart ESP32-CAM "+restartCount+" times.";
        myTimer = setInterval(function(){getStill.click();},10000);
      }
      else
        // Menampilkan pesan error jika telah mencoba restart lebih dari 2 kali
        result.innerHTML = "Get still error. <br>Please close the page and check ESP32-CAM.";
    }     

    // Event handler saat gambar di ShowImage selesai dimuat
    ShowImage.onload = function (event) {
      clearInterval(myTimer);
      restartCount=0;      
      // Mengatur ukuran canvas sesuai dengan ukuran gambar yang dimuat
      canvas.setAttribute("width", ShowImage.width);
      canvas.setAttribute("height", ShowImage.height);
      
      if (mirrorimage.value==1) {
        // Melakukan transformasi gambar jika opsi mirrorimage bernilai 1
        context.translate((canvas.width + ShowImage.width) / 2, 0);
        context.scale(-1, 1);
        context.drawImage(ShowImage, 0, 0, ShowImage.width, ShowImage.height);
        context.setTransform(1, 0, 0, 1, 0, 0);
      }
      else
        // Menggambar gambar tanpa transformasi jika mirrorimage tidak bernilai 1
        context.drawImage(ShowImage,0,0,ShowImage.width,ShowImage.height);
      if (Model) {
        // Memanggil fungsi DetectImage setelah gambar dimuat
        DetectImage();
      }          
    }     
    
    // Event handler saat tombol "restart" diklik
    restart.onclick = function (event) {
      fetch(location.origin+'/?restart=stop');
    }    

    // Event handler saat opsi "framesize" diubah
    framesize.onclick = function (event) {
      fetch(document.location.origin+'/?framesize='+this.value+';stop');
    }  

    // Event handler saat opsi "flash" diubah
    flash.onchange = function (event) {
      fetch(location.origin+'/?flash='+this.value+';stop');
    } 

    // Event handler saat opsi "quality" diubah
    quality.onclick = function (event) {
      fetch(document.location.origin+'/?quality='+this.value+';stop');
    } 

    // Event handler saat opsi "brightness" diubah
    brightness.onclick = function (event) {
      fetch(document.location.origin+'/?brightness='+this.value+';stop');
    } 

    // Event handler saat opsi "contrast" diubah
    contrast.onclick = function (event) {
      fetch(document.location.origin+'/?contrast='+this.value+';stop');
    }                             
    
    // Fungsi untuk memulai deteksi objek
    function ObjectDetect() {
      // Menampilkan pesan saat model sedang dimuat
      result.innerHTML = "Please wait for loading model.";
      // Memuat model COCO-SSD
      cocoSsd.load().then(cocoSsd_Model => {
        Model = cocoSsd_Model;
        result.innerHTML = "";
        getStill.style.display = "block";
      }); 
    }
    
    // Fungsi untuk mendeteksi objek pada gambar
    function DetectImage() {
      Model.detect(canvas).then(Predictions => {    
        var s = (canvas.width>canvas.height)?canvas.width:canvas.height;
        var objectCount=0;  // Menghitung jumlah objek yang terdeteksi
        var trackState = 0;  // Menandai apakah objek sudah terdeteksi
        var x, y, width, height;
        
        // Menggambar kotak persegi di sekitar objek yang terdeteksi
        if (Predictions.length>0) {
          result.innerHTML = "";
          for (var i=0;i<Predictions.length;i++) {
            const x = Predictions[i].bbox[0];
            const y = Predictions[i].bbox[1];
            const width = Predictions[i].bbox[2];
            const height = Predictions[i].bbox[3];
            context.lineWidth = Math.round(s/200);
            context.strokeStyle = "#00FFFF";
            context.beginPath();
            context.rect(x, y, width, height);
            context.stroke(); 
            context.lineWidth = "2";
            context.fillStyle = "red";
            context.font = Math.round(s/30) + "px Arial";
            context.fillText(Predictions[i].class, x, y);
            result.innerHTML+= "[ "+i+" ] "+Predictions[i].class+", "+Math.round(Predictions[i].score*100)+"%, "+Math.round(x)+", "+Math.round(y)+", "+Math.round(width)+", "+Math.round(height)+"<br>";

            // Melakukan tracking objek jika sesuai dengan kriteria
            if (Predictions[i].class==object.value&&Predictions[i].score>=score.value&&trackState==0) {   
              trackState = 1;  // Hanya melakukan tracking pada objek pertama
              
              // Menghitung titik tengah horizontal objek yang terdeteksi
              var midX = Math.round(x)+Math.round(width)/2;  
              if (midX>(40+320/2)) {  
                if (midX>260) {  
                  if (mirrorimage.value==1) {angle1Value1-=350;}else{angle1Value1+=350;}
                } else {
                  if (mirrorimage.value==1) {angle1Value1-=175;}else{angle1Value1+=175;}
                }
                if (angle1Value1 > 7650) angle1Value1 = 7650;
                if (angle1Value1 < 2050) angle1Value1 = 2050;
                $.ajax({url: document.location.origin+'?servo1='+angle1Value1, async: false}); 
              }
              else if (midX<(320/2)-40) {  
                if (midX<60) {  
                  if (mirrorimage.value==1) {angle1Value1+=350;}else{angle1Value1-=350;}
                } else {
                  if (mirrorimage.value==1) {angle1Value1+=175;}else{angle1Value1-=175;}
                }
                if (angle1Value1 > 7650) angle1Value1 = 7650;
                if (angle1Value1 < 2050) angle1Value1 = 2050;                 
                $.ajax({url: document.location.origin+'?servo1='+angle1Value1, async: false}); 
              }
                
              // Menghitung titik tengah vertikal objek yang terdeteksi
              var midY = Math.round(y)+Math.round(height)/2;  
              if (midY>(240/2+30)) {  
                if (midY>195) {  
                  angle1Value2-=300;
                } else {
                  angle1Value2-=150; 
                }
                if (angle1Value2 > 7650) angle1Value2 = 7650;
                if (angle1Value2 < 2050) angle1Value2 = 2050;                  
                $.ajax({url: document.location.origin+'?servo2='+angle1Value2, async: false}); 
              }
              else if (midY<(240/2)-30) {  
                if (midY<45) {  
                  angle1Value2+=300;
                } else {
                  angle1Value2+=150;   
                }
                if (angle1Value2 > 7650) angle1Value2 = 7650;
                if (angle1Value2 < 2050) angle1Value2 = 2050;                  
                $.ajax({url: document.location.origin+'?servo2='+angle1Value2, async: false});  
              }    
            }
                      
            if (Predictions[i].class==object.value) {
              objectCount++;
            }  
          }
          count.innerHTML = objectCount;
        }
        else {
          result.innerHTML = "Unrecognizable";
          count.innerHTML = "0";
        }
    
        try { 
          document.createEvent("TouchEvent");
          setTimeout(function(){getStill.click();},250);
        }
        catch(e) { 
          setTimeout(function(){getStill.click();},150);
        } 
      });
    }

    function getFeedback(target) {
      var data = $.ajax({
      type: "get",
      dataType: "text",
      url: target,
      success: function(response)
        {
          result.innerHTML = response;
        },
        error: function(exception)
        {
          result.innerHTML = 'fail';
        }
      });
    }     

    // Memanggil fungsi ObjectDetect saat halaman dimuat
    window.onload = function () { ObjectDetect(); }    
</script>
 </body>
</html> 
)rawliteral";

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // Menonaktifkan setelan restart saat tegangan tidak stabil
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);  // Mengaktifkan output debug
  Serial.println();

  // Konfigurasi kamera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  // Inisialisasi dengan spesifikasi tinggi untuk menyiapkan buffer yang lebih besar
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63, semakin rendah semakin berkualitas
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63, semakin rendah semakin berkualitas
    config.fb_count = 1;
  }
  
  // Inisialisasi kamera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Inisialisasi kamera gagal dengan kesalahan 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  // Mengurangi ukuran frame untuk meningkatkan laju frame awal
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA  Setel resolusi gambar awal
  
  // Inisialisasi servo1
  ledcAttachPin(12, 3);  
  ledcSetup(3, 50, 16);
  ledcWrite(3, 4850);
  delay(1000);

  // Inisialisasi servo2
  ledcAttachPin(13, 5);  
  ledcSetup(5, 50, 16);
  ledcWrite(5, 4850);  
  delay(1000);
  
  // Inisialisasi LED Flash
  ledcAttachPin(14, 4);  
  ledcSetup(4, 5000, 8);    
  
  WiFi.mode(WIFI_AP_STA);
  
  // Mulai koneksi jaringan
  // Mengatur IP statis untuk klien
  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));
  WiFi.begin(ssid, password);    

  delay(1000);
  Serial.println("");
  Serial.print("Menghubungkan ke ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;    // Tunggu koneksi selama 10 detik
  } 

  if (WiFi.status() == WL_CONNECTED) {    // Jika koneksi berhasil
    // Atur SSID dengan menampilkan IP klien
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);  
    Serial.println("");
    Serial.println("Alamat IP STA: ");
    Serial.println(WiFi.localIP());  

    for (int i=0;i<5;i++) {   // Jika terhubung ke WIFI, kedipan LED Flash
      ledcWrite(4,10);
      delay(200);
      ledcWrite(4,0);
      delay(200);    
    }         
  }
  else {
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);         

    for (int i=0;i<2;i++) {    // Jika tidak terhubung ke WIFI, kedipan LED Flash lebih lambat
      ledcWrite(4,10);
      delay(1000);
      ledcWrite(4,0);
      delay(1000);    
    }
  }     

  // Mengatur IP AP
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("Alamat IP AP: ");
  Serial.println(WiFi.softAPIP());    

  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);  

  server.begin();          
}

void loop() {
  // Variabel untuk menyimpan feedback dan perintah
  Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  // Status variabel
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
   // Menerima klien WiFi
  WiFiClient client = server.available();

  if (client) { 
    // Inisialisasi string untuk menyimpan data dari klien
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        // Membaca karakter yang tersedia dari klien
        char c = client.read();             
        
        // Memproses karakter untuk mendapatkan perintah
        getCommand(c);   // Memecah karakter yang diterima dari buffer menjadi parameter perintah
                
        // Jika menemui akhir baris
        if (c == '\n') {
          // Jika baris saat ini kosong
          if (currentLine.length() == 0) {    
            
            // Jika perintah adalah "getstill"
            if (cmd=="getstill") {
              // Mengambil gambar dalam format JPEG
              camera_fb_t * fb = NULL;
              fb = esp_camera_fb_get();  
              if(!fb) {
                Serial.println("Pengambilan gambar kamera gagal");
                delay(1000);
                ESP.restart();
              }
  
              // Mengirim header HTTP untuk gambar
              client.println("HTTP/1.1 200 OK");
              client.println("Access-Control-Allow-Origin: *");              
              client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
              client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
              client.println("Content-Type: image/jpeg");
              client.println("Content-Disposition: form-data; name=\"imageFile\"; filename=\"picture.jpg\""); 
              client.println("Content-Length: " + String(fb->len));             
              client.println("Connection: close");
              client.println();
              
              // Mengirim data gambar
              uint8_t *fbBuf = fb->buf;
              size_t fbLen = fb->len;
              for (size_t n=0;n<fbLen;n=n+1024) {
                if (n+1024<fbLen) {
                  client.write(fbBuf, 1024);
                  fbBuf += 1024;
                }
                else if (fbLen%1024>0) {
                  size_t remainder = fbLen%1024;
                  client.write(fbBuf, remainder);
                }
              }  
              
              // Melepaskan buffer gambar
              esp_camera_fb_return(fb);
            
              // Mematikan LED Flash
              pinMode(14, OUTPUT);
              digitalWrite(14, LOW);               
            }
            else {
              // Mengirim halaman HTML utama atau Feedback
              client.println("HTTP/1.1 200 OK");
              client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
              client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
              client.println("Content-Type: text/html; charset=utf-8");
              client.println("Access-Control-Allow-Origin: *");
              client.println("Connection: close");
              client.println();
              
              String Data="";
              if (cmd!="")
                Data = Feedback;
              else {
                Data = String((const char *)INDEX_HTML);
              }
              int Index;
              for (Index = 0; Index < Data.length(); Index = Index+1000) {
                client.print(Data.substring(Index, Index+1000));
              }           
              
              client.println();
            }
                        
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }

        // Jika baris mengandung permintaan HTTP
        if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1)) {
          // Jika perintah mengandung kata kunci "stop", putuskan koneksi segera -> http://192.168.xxx.xxx/?cmd=aaa;bbb;ccc;stop
          if (Command.indexOf("stop")!=-1) {  
            client.println();
            client.println();
            client.stop();
          }
          currentLine="";
          Feedback="";
          ExecuteCommand();
        }
      }
    }
    // Memberi jeda sejenak sebelum menutup koneksi
    delay(1);
    client.stop();
  }
}
