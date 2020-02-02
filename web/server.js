const express = require('express');
var HID = require('node-hid');
const os = require('os');

const app = express();
const port = 3000;
let device;

app.use(express.static('public'));
app.use(express.json());

app.post('/tv', (req, res) => {
  const teensy = HID.devices().find(device => {
    return device.vendorId == 0x16C0 && device.usage === 0x200;
  });
  if (teensy) {
    device = new HID.HID(teensy.path);
  }
  else {
    console.log('No Teensy found');
    res.sendStatus(404);
    return;
  }
  sendBytesToDevice(req.body);
  res.sendStatus(200);
});

app.listen(port, () => {
  console.log(`App listening on port ${port}`);
});

function sendBytesToDevice(bytes) {
  const packets = bytes.reduce((acc, byte, index) => {
    if (index % 64 === 0) {
      if (os.platform() === 'win32') {
        return [...acc, [0x00, byte]];
      }
      else {
        return [...acc, [byte]];
      }
    }
    const currPacket = acc[acc.length-1];
    currPacket.push(byte);
    return acc;
  }, []);
  packets.forEach(packet => {
    const sentLength = device.write(packet);
  });
}
