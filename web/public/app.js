const can = document.querySelector('canvas');
const ctx = can.getContext('2d');
can.width = 52;
can.height = 39;
// ctx.imageSmoothingEnabled = false;

const fileInput = document.querySelector('input');

can.addEventListener('dragenter', dragEnterHandler, false);
can.addEventListener('dragleave', dragLeaveHandler, false);
can.addEventListener('dragover', dragOverHandler, false);
can.addEventListener('drop', dropHandler, false);

fileInput.addEventListener('change', uploadHandler);

function dragEnterHandler(e) {
  e.preventDefault();
  e.target.classList.add('dragging');
}

function dragLeaveHandler(e) {
  e.preventDefault();
  e.target.classList.remove('dragging');
}

function dragOverHandler(e) {
  e.preventDefault();
  return false;
}

function uploadHandler(e) {
  const { files } = e.target;
  loadImage(files[0]);
}

function dropHandler(e) {
  e.preventDefault();
  e.target.classList.remove('dragging');
  const { files } = e.dataTransfer;
  loadImage(files[0]);
}

function loadImage(file) {
  const img = new Image;
  img.onload = () => {
    // clear canvas
    ctx.fillStyle = 'black';
    ctx.fillRect(0,0,can.width,can.height);
    const w = img.width;
    const h = img.height;
    // fill canvas
    const ratio = Math.max(can.width / w, can.height / h);
    const dw = ratio * w;
    const dh = ratio * h;
    const dx = (can.width - w * ratio) / 2;
    const dy = (can.height - h * ratio) / 2;
    ctx.drawImage(img, 0, 0, w, h, dx, dy, dw, dh);
    // convert to grayscale
    processBitmap();
  };
  img.src = URL.createObjectURL(file);
}

function processBitmap() {
  const bytes = [];
  const imgData = ctx.getImageData(0,0,can.width, can.height);
  for(let i=0; i< imgData.data.length; i+=4) {
    const r = imgData.data[i];
    const g = imgData.data[i+1];
    const b = imgData.data[i+2];
    const luma = Math.floor(16 * (0.299*r/255 + 0.587*g/255 + 0.114*b/255));
    imgData.data[i] = imgData.data[i+1] = imgData.data[i+2] = 16 * luma;
    bytes.push(luma);
  }
  ctx.putImageData(imgData, 0, 0);
  // send to server
  fetch('/tv', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(bytes)
  })
    .catch(err => {
      console.error(err);
    });
}
