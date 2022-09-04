let btnForward = document.querySelector('#btnForward');

btnForward.addEventListener('click',() => btnForward.style.backgroundColor='#337ab7')







async function left() {
    await fetch("left", { method: "POST" });
}

async function forward() {
    document.getElementById("btn").style.color = "black";
    await fetch("forward", { method: "POST" });
}

async function right() {
    await fetch("right", { method: "POST" });
}

async function back() {
    await fetch("backward", { method: "POST" });
}

async function stop() {
    await fetch("stop", { method: "POST" });
}

