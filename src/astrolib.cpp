#include "headers/astrolib.h"
#include "sweodef.h"
#include "swephexp.h"

#include <QProcess>
#include <QTimeZone>
#include <QSettings>

namespace AL {

  void init() {
    char path[] = "/home/spagyricus/projects/AstroLib/libswe/ephem";
    swe_set_ephe_path(path);
    
    QSettings settings;
    debug(settings.value("paths/astrolog").toString());
   /* alTz = tz;
    alLat = lat;
    alLong = longitude;*/
    
    // build hashes
  /*  alPlanetsHash["Satu"] = Saturn;
    alPlanetsHash["Jupi"] = Jupiter;
    alPlanetsHash["Mars"] = Mars;
    alPlanetsHash["Sun"] = Sun;
    alPlanetsHash["Venu"] = Venus;
    alPlanetsHash["Merc"] = Mercury;
    alPlanetsHash["Moon"] = Moon;
    alPlanetsHash["Uran"] = Uranus;
    alPlanetsHash["Nept"] = Neptune;
    alPlanetsHash["Plut"] = Pluto;
    
    alZodiacHash["Ari"] = Aries;
    alZodiacHash["Tau"] = Taurus;
    alZodiacHash["Gem"] = Gemini;
    alZodiacHash["Can"] = Cancer;
    alZodiacHash["Leo"] = Leo;
    alZodiacHash["Vir"] = Virgo;
    alZodiacHash["Lib"] = Libra;
    alZodiacHash["Sco"] = Scorpio;
    alZodiacHash["Sag"] = Sagittarius;
    alZodiacHash["Cap"] = Capricorn;
    alZodiacHash["Aqu"] = Aquarius;
    alZodiacHash["Pis"] = Pisces;*/
  }
  
  QList<QPair<QDateTime,QDateTime>> voids(const QDate &day)
  {
    QStringList daylines = astrolog_getVoids(day);
    QList<QPair<QDateTime,QDateTime>> theVoids;
    
    for (int i=0; i<daylines.size(); ++i) {
      QStringList cols = daylines.at(i).split(QRegExp("\\s+"));
      if (cols.at(3) == "Moon" && cols.at(5) == "-->") { 
        // void end found!
        // now find where it started
        QStringList scols;
        int j=i-1;
        bool found = false;
        while (j>=0) {
          scols = daylines.at(j).split(QRegExp("\\s+"));
          if (scols.at(3) == "Moon") {
            // found it!
            found = true;
            break;
          }
          --j;
        }
        int cnt = 0; // how far back in days to serach.
        while (!found) {
          // search through previous days until we find the start.
          cnt++;
          QStringList prevlines = astrolog_getVoids(day.addDays(0-cnt));
          j=prevlines.size()-1;
          while (j>=0) {
            scols = prevlines.at(j).split(QRegExp("\\s+"));
            if (scols.at(3) == "Moon") {
              // found it!
              found = true;
              break;
            }
            --j;
          }
          if (cnt>5) {
            // bail out after 5 days, it'll never be that long.
            err("Something went wrong with astrolog parsing...");
            assert(found); // it absolutely should be found by now.
            break;
          }
        }
        
        // add both to map.
        QDateTime startdt, enddt;
        QString date = scols.at(1);
        date.remove(QRegExp("\\s+"));
        startdt.setDate(QDate::fromString(date, "M/d/yyyy"));
        startdt.setTime(QTime::fromString(scols.at(2), "h:mmap"));
        date = cols.at(1);
        date.remove(QRegExp("\\s+"));
        enddt.setDate(QDate::fromString(date, "M/d/yyyy"));
        enddt.setTime(QTime::fromString(cols.at(2), "h:mmap"));
        theVoids << QPair<QDateTime,QDateTime>(startdt, enddt);
      }
    }
    
    // now find next moon transit in future (starting next day) and if it's a void end, find its start.
    bool found = false;
    int cnt = 0;
    while (!found) {
      cnt++;
      QStringList nextlines = astrolog_getVoids(day.addDays(cnt));
      for (int k=0; k<nextlines.size(); ++k) {
        QStringList cols = nextlines.at(k).split(QRegExp("\\s+"));
        if (cols.at(3) == "Moon") {
          if (cols.at(5) != "-->") { 
            // Okay we're good, no void started yesterday, break loops and finish.
            found = true;
            break;
          }
          else {
            // void end found!
            // now find where it started
            found = true; // break outer while loop after this.
            bool startFound = false;
            cnt = 0;
            QStringList scols;
            while (!startFound) {
              // search through previous days until we find the start.
              QStringList prevlines = ( cnt==0 ) ? daylines : astrolog_getVoids(day.addDays(0-cnt));
              int j=prevlines.size()-1;
              while (j>=0) {
                scols = prevlines.at(j).split(QRegExp("\\s+"));
                if (scols.at(3) == "Moon") {
                  // found it!
                  startFound = true;
                  break;
                }
                --j;
              }
              if (!startFound && cnt>5) {
                // bail out after 5 days, it'll never be that long.
                err("Something went wrong with astrolog parsing...");
                assert(startFound); // it absolutely should be found by now.
                break;
              }
              cnt++;
            }
            
            // add both to map.
            QDateTime startdt, enddt;
            QString date = scols.at(1);
            date.remove(QRegExp("\\s+"));
            startdt.setDate(QDate::fromString(date, "M/d/yyyy"));
            startdt.setTime(QTime::fromString(scols.at(2), "h:mmap"));
            date = cols.at(1);
            date.remove(QRegExp("\\s+"));
            enddt.setDate(QDate::fromString(date, "M/d/yyyy"));
            enddt.setTime(QTime::fromString(cols.at(2), "h:mmap"));
            theVoids << QPair<QDateTime,QDateTime>(startdt, enddt);
            
            break; // break inner for loop now that we've found it
          }
        }
      }
    }
    debug(QString("%1 VOIDS FOUND").arg(theVoids.count()));
    return theVoids;
  }
  
  QPair<QDateTime, QDateTime> riseSet(const QDate& date)
  {
    return riseSet(date, date).first();
  }
  
  QList<QPair<QDateTime, QDateTime>> riseSet(const QDate& from, const QDate& to) 
  {
    QList<QPair<QDateTime, QDateTime>> dlist;
    QDate cdate = from;
    while (cdate <= to) {
      char serr[AS_MAXCH];
      double epheflag = SEFLG_SWIEPH;
      int gregflag = SE_GREG_CAL;
      
      int year = cdate.year();
      int month = cdate.month();
      int day = cdate.day();
      
      double geo_longitude = -105.097938; // positive for east, negative for west of Greenwich
      //double geo_longitude = alLong.toDouble(); // positive for east, negative for west of Greenwich
      double geo_latitude = 40.044488;
      //double geo_latitude = alLat.toDouble();
      double geo_altitude = 1564.0; ///FIXME is this meters??
      
      double rhour;
      double shour;
      
      // array for atmospheric conditions
      double datm[2];
      datm[0] = 1013.25; // atmospheric pressure;
      // irrelevant with Hindu method, can be set to 0
      datm[1] = 15;      // atmospheric temperature;
      // irrelevant with Hindu method, can be set to 0
      
      double geopos[3];
      geopos[0] = geo_longitude;
      geopos[1] = geo_latitude;
      geopos[2] = geo_altitude;
      
      swe_set_topo(geopos[0], geopos[1], geopos[2]);
      
      int ipl = SE_SUN; // object whose rising is wanted
      char starname[255]; // name of star, if a star's rising is wanted
      // is "" or NULL, if Sun, Moon, or planet is calculated
      double trise; // for rising time
      double tset;  // for setting time
      
      // calculate the julian day number of the date at 0:00 UT:
      double tjd = swe_julday(year,month,day,0,gregflag);
      
      // convert geographic longitude to time (day fraction) and subtract it from tjd
      // this method should be good for all geographic latitudes except near in
      // polar regions
      double dt = geo_longitude / 360.0;
      tjd = tjd - dt;
      
      int rsmi = SE_CALC_RISE;
      int return_code = swe_rise_trans(tjd, ipl, starname, epheflag, rsmi, geopos, datm[0], datm[1], &trise, serr);
      
      if (return_code == ERR) {
        err(serr);
        return QList<QPair<QDateTime, QDateTime>>();
      }
      
      swe_revjul(trise, gregflag, &year, &month, &day, &rhour);
      
      
      rsmi = SE_CALC_SET;
      return_code = swe_rise_trans(trise, ipl, starname, epheflag, rsmi, geopos, datm[0], datm[1], &tset, serr);
      
      if (return_code == ERR) {
        err(serr);
        return QList<QPair<QDateTime, QDateTime>>();
      }
      
      swe_revjul(tset, gregflag, &year, &month, &day, &shour);
     // printf("Day length: %f\tJulian days\nDay length: %f\tHours\n\n", (tset - trise), (tset - trise) * 24 );
      
      // Turn Julian day into first UTC then localtime
      int iyear_utc;
      int imonth_utc;
      int iday_utc;
      int ihour_utc;
      int imin_utc;
      double dsec_utc;
      
      double d_timezone = -7;
      int iyear;
      int imonth;
      int iday;
      int ihour;
      int imin;
      double dsec;
      
      QPair<QDateTime,QDateTime> pair;
      
      swe_jdet_to_utc(trise, gregflag, &iyear_utc, &imonth_utc, &iday_utc, &ihour_utc, &imin_utc, &dsec_utc);
     // printf("UTC   sunrise : date=%i/%i/%i, time=%02i:%02i:%05.2f\n", iyear_utc, imonth_utc, iday_utc, ihour_utc, imin_utc, dsec_utc);
      
      swe_utc_time_zone(iyear_utc, imonth_utc, iday_utc, ihour_utc, imin_utc, dsec_utc, -d_timezone, &iyear, &imonth, &iday, &ihour, &imin, &dsec);
     // printf("Local sunrise  : date=%i/%i/%i, time=%02i:%02i:%05.2f\n\n", iyear, imonth, iday, ihour, imin, dsec);
      pair.first.setDate(QDate(iyear,imonth,iday));
      pair.first.setTime(QTime(ihour,imin,dsec));
      
      swe_jdet_to_utc(tset, gregflag, &iyear_utc, &imonth_utc, &iday_utc, &ihour_utc, &imin_utc, &dsec_utc);
    //  printf("UTC   sunset  : date=%i/%i/%i, time=%02i:%02i:%05.2f\n", iyear_utc, imonth_utc, iday_utc, ihour_utc, imin_utc, dsec_utc);
      
      swe_utc_time_zone(iyear_utc, imonth_utc, iday_utc, ihour_utc, imin_utc, dsec_utc, -d_timezone, &iyear, &imonth, &iday, &ihour, &imin, &dsec);
    //  printf("Local sunset  : date=%i/%i/%i, time=%02i:%02i:%05.2f\n\n", iyear, imonth, iday, ihour, imin, dsec);
     
      pair.second.setDate(QDate(iyear,imonth,iday));
      pair.second.setTime(QTime(ihour,imin,dsec));
      // adjust for DST
      if (pair.first.isDaylightTime()) pair.first = pair.first.addSecs(60*60*1);
      if (pair.second.isDaylightTime()) pair.second = pair.second.addSecs(60*60*1);
      dlist << pair;
      
      cdate = cdate.addDays(1);
    }
    
    return dlist;
  }
  
  QStringList astrolog_getVoids(const QDate& day)
  {
    QSettings settings;
    QProcess *al = new QProcess();
    QString cmd = settings.value("paths/astrolog").toString();
    QStringList args;
    // astrolog -qb 8 16 2018 12:00 1 MT 105W05:51 40N02:39 -dm
    args << "-qb" << QString::number(day.month()) << QString::number(day.day()) << QString::number(day.year()) << "12:00";
    QDateTime dt;
    dt.setDate(day);
    dt.setTime(QTime(12,0,0));
    args << QString( (dt.isDaylightTime()) ? "1" : "0");
    args << settings.value("locale/tz").toString();
    args << settings.value("locale/long").toString();
    args << settings.value("locale/lat").toString();
    args << "-d"; // get transits for the day including void of course.
    al->start(cmd, args);
    if (!al->waitForFinished()) {
      err("Something went wrong calling astrolog!!");
      assert(false);
    }
    
    QStringList lines;
    while (al->canReadLine()) {
      QString line = al->readLine();
      // fix whitespace padded dates! eg: "11/ 3/1999" 
      QRegExp rx;
      rx.setPattern("\\s+(\\d+)/\\s*(\\d+)/(\\d+)\\s+");
      if (line.contains(rx)) // line.contains causes rx captures to happen, while line.replace alone does not!
        line.replace(rx, QString(" %1/%2/%3 ").arg(rx.cap(1), rx.cap(2), rx.cap(3)));
      lines << line;
    }
    
    return lines;
  }
  
  QStringList astrolog_getChart(const QDateTime& dt)
  {
    QSettings settings;
    QProcess *al = new QProcess();
    QString cmd = settings.value("paths/astrolog").toString();
    QStringList args;
    // astrolog -qb 8 16 2018 12:00 1 MT 105W05:51 40N02:39 -dm
    args << "-qb" 
    << QString::number(dt.date().month()) 
    << QString::number(dt.date().day()) 
    << QString::number(dt.date().year()) 
    << dt.time().toString("HH:mm:ss")
    << QString( (dt.isDaylightTime()) ? "1" : "0")
    << settings.value("locale/tz").toString()
    << settings.value("locale/long").toString()
    << settings.value("locale/lat").toString()
    << "-v"; // get normal chart output
    al->start(cmd, args);
    if (!al->waitForFinished()) {
      err("Something went wrong calling astrolog!!");
      assert(false);
    }
    
    QStringList lines;
    while (al->canReadLine()) {
      lines << al->readLine();
    }
    
    return lines;
  }
  
} // namespace AL





/*
 / /*  Calculate Nocturnal Tides
 int i;
 int j;
 long double nocday = ( tset - trise ) / 30 ;
 printf("nocday: %Lf\n\n", nocday);
 for ( i=0; i<30; i++) {
   
   j = i % 5;
   
   switch(j){
     case 0 :
       printf("Spirit:\t");
       break;
     case 1 :
       printf("Air:\t");
       break;
     case 2 :
       printf("Fire:\t");
       break;
     case 3 :
       printf("Water:\t");
       break;
     case 4 :
       printf("Earth:\t");
       break;
       }
       
       printf("%Lf\t", trise + i * nocday);
       
       swe_jdet_to_utc(trise + i * nocday, gregflag, &iyear_utc, &imonth_utc, &iday_utc, &ihour_utc, &imin_utc, &dsec_utc);
       swe_utc_time_zone(iyear_utc, imonth_utc, iday_utc, ihour_utc, imin_utc, dsec_utc, -d_timezone, &iyear, &imonth, &iday, &ihour, &imin, &dsec);
       printf(": date=%i/%i/%i, time=%02i:%02i:%05.2f\n", iyear, imonth, iday, ihour, imin, dsec);
       
       }
       printf("\nCheck: %Lf\n\n", trise + 29 * nocday );
       
       // Calculate moon stuff. Needs more work
       double attr[20];
       return_code = swe_pheno_ut( tjd, SE_MOON, 0, attr, serr);
       
       if (return_code == ERR) {
         printf("%s\n", serr);
         }
         
         printf("Moon phase: %.2f%%  ", attr[1] * 100);
         
         double today = attr[1];
         return_code = swe_pheno_ut( tjd - 1, SE_MOON, 0, attr, serr);
         
         if (return_code == ERR) {
           // error action
           printf("%s\n", serr);
           }
           
           if ( today > attr[1] ) {
             printf("Waxing\n");
             }
             else {
               printf("Waning\n");
               }
               
               printf("Moon phase angle: %f\n", attr[0]);
               
               printf("\n");
               */
