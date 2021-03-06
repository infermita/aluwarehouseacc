#include "nfcthread.h"
#include "lib/httpclient.h"
#include <QDebug>
#include <QNetworkInterface>
#include "wiringPi.h"
#include "writelcd.h"

NfcThread::NfcThread()
{

}
void NfcThread::run(){

    const nfc_modulation nmMifare = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };
    QString id,url,ip,read,lcd;
    int pin = 0;
    HttpClient http;
    int i;
    bool ipcheck = true;
    QStringList myOptions;
    myOptions << "SI" << "NO";

    wiringPiSetup();
    pinMode (pin, OUTPUT) ;

    WriteLcd *wLcd = new WriteLcd();

    wLcd->clear();
    wLcd->write(0,0,"ATTESA RETE");

    while(ipcheck){

        ip = QNetworkInterface::interfaceFromName("wlan0").addressEntries().first().ip().toString();

        qDebug() << "Attesa rete con ip: " << ip << " count: " << QString::number(ip.split(".").count());

        if(ip.split(".").count()==4)
            ipcheck = false;
        else
            sleep(1);
    }

    while(1){
         nfc_init(&context);
         pnd = nfc_open(context, NULL);
         if (pnd == NULL) {
             qDebug() << "ERROR: %s. Unable to open NFC device.";

         }else{

             qDebug() << "Nfc aperto";

             if (nfc_initiator_init(pnd) < 0) {
                 qDebug() << "nfc_initiator_init";
                 nfc_close(pnd);
                 nfc_exit(context);

             }else{
                 qDebug() << "Nfc iniator";

                 wLcd->clear();
                 wLcd->write(0,0,"ATTESA BADGE");

                 if(nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) > 0){
                     id = "";
                     for(i = 0; i < nt.nti.nai.szUidLen;i++){

                         QString hex;
                         id += hex.sprintf("%02x",nt.nti.nai.abtUid[i]).toUpper();

                     }
                     qDebug() << "Leggo: " << id;
                     url = "/default/servicesonde/waccess/cardkeyw/"+id;
                     read = http.Get(url);

                     qDebug() << "Per: " << id << "leggo: " << read;

                     switch (myOptions.indexOf(read)) {
                         case 0:
                             lcd = read+" PUOI ENTRARE";
                             wLcd->write(0,0,lcd.toUtf8().data());

                             digitalWrite (pin, HIGH) ;
                             delay (500) ;
                             digitalWrite (pin, LOW);
                             break;
                         case 1:
                             read += "N";
                             lcd = read+" PUOI ENTRARE";
                             wLcd->write(0,0,lcd.toUtf8().data());
                             break;
                         default:
                             lcd = "ERRORE SERVER";
                             wLcd->write(0,0,lcd.toUtf8().data());
                             break;
                     }

                     while(!nfc_initiator_target_is_present(pnd,&nt)){
                         sleep(1);
                     }
                     nfc_close(pnd);
                     nfc_exit(context);
                     delay (500) ;

                 }
             }
         }
     }

}
