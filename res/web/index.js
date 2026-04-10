const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
const fpsElement = document.getElementById('fps');

let particles = [];
let frames = 0;
let lastTime = performance.now();

function resize() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
}
window.addEventListener('resize', resize);
resize();

class Particle {
    constructor() {
        this.x = Math.random() * canvas.width;
        this.y = Math.random() * canvas.height;
        this.vx = (Math.random() - 0.5) * 6;
        this.vy = (Math.random() - 0.5) * 6;
        this.radius = Math.random() * 15 + 5;
        this.hue = Math.random() * 360;
    }
    update() {
        this.x += this.vx;
        this.y += this.vy;
        if (this.x - this.radius < 0 || this.x + this.radius > canvas.width) this.vx *= -1;
        if (this.y - this.radius < 0 || this.y + this.radius > canvas.height) this.vy *= -1;
        this.hue += 2;
    }
    draw() {
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fillStyle = `hsl(${this.hue}, 100%, 60%)`;
        ctx.shadowBlur = 15;
        ctx.shadowColor = ctx.fillStyle;
        ctx.fill();
        ctx.closePath();
    }
}

for (let i = 0; i < 60; i++) particles.push(new Particle());

function animate(time) {
    ctx.fillStyle = 'rgba(26, 26, 46, 0.3)';
    ctx.shadowBlur = 0;
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    particles.forEach(p => {
        p.update();
        p.draw();
    });

    frames++;
    if (time - lastTime >= 1000) {
        fpsElement.innerText = frames;
        frames = 0;
        lastTime = time;
    }

    requestAnimationFrame(animate);
}
animate(performance.now());