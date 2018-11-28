const express = require('express');
const bodyParser = require('body-parser');
const app = express();
const port = process.env.PORT || 3000;

const admin = require('firebase-admin');

const serviceAccount = require('link to the database');

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: 'link to the database url'
});

const ref_room = admin.database().ref('room');

app.set('json spaces', 4);
app.use(bodyParser.json());

app.route('/room')
  .get((req, res) => {
    const from = Number(req.param("from"));
    const to = Number(req.param("to"));
    const sensor = req.param("sensor");

    ref_room.child(sensor)
      .orderByChild("timestamp")
      .startAt(from)
      .endAt(to)
      .once("value", (data) => {})
      .then((value) => res.json(value));
  })
  .post((req, res) => {

    const ref_hum = ref_room.child("humidity");
    const ref_pre = ref_room.child("presence");
    const ref_tem = ref_room.child("temperature");
    const ref_smo = ref_room.child("smoke");
    const ref_int = ref_room.child("intruder");

    const time_now = Date.now();

    if (req.body.humidity){
      ref_hum.push({...req.body.humidity, timestamp: time_now});
    }
    if (req.body.presence){
      ref_pre.push({...req.body.presence, timestamp: time_now});
    }
    if (req.body.temperature){
      ref_tem.push({...req.body.temperature, timestamp: time_now});
    }
    if (req.body.smoke){
      ref_smo.push({...req.body.smoke, timestamp: time_now});
    }
    if (req.body.intruder){
      ref_int.push({...req.body.intruder, timestamp: time_now});
    }
    
    res.send("Data inserted.");
  })
  .put((req, res) => {
    ref_room.update(req.body);
    
    res.send("Light/temperature updated.");
  })
  .delete((req, res) => {
    res.send("Not implemented yet.");
  });


app.listen(port, () => console.log(`Server listening on port ${port}!`));
