const num_rows = 6;
const num_cols = 5;

let current_row = 0;
let current_col = 0;
let finished = false;
let field = document.getElementById("field");
let ws = new WebSocket("ws://localhost:3000/random");

function isLetter(str) {
    return str.length === 1 && ((str >= 'a' && str <= 'z') || (str >= 'A' && str <= 'Z'))
}

function initField() {
    field.innerHTML = "";
    for (let i = 0; i < num_rows; ++i) {
        let row = document.createElement("div");
        row.classList.add("row");
        for (let j = 0; j < num_cols; ++j) {
            let letterbox = document.createElement("div");
            letterbox.classList.add("letterbox");
            letterbox.classList.add("blank");
            row.appendChild(letterbox);
        }
        field.appendChild(row);
    }
}

function getLetterBox(row, col) {
    return field.children[row].children[col];
}

initField();

window.onkeydown = (event) => {
    if (finished) {
        return;
    }
    if (isLetter(event.key) && current_row < num_rows && current_col < num_cols) {
        let letterbox = getLetterBox(current_row, current_col);
        letterbox.classList.remove("blank");
        letterbox.classList.add("focus");
        letterbox.innerHTML = event.key.toUpperCase();
        ++current_col;
    } else if (event.key === "Backspace" && current_row < num_rows && current_col > 0) {
        let letterbox = getLetterBox(current_row, current_col - 1);
        letterbox.classList.remove("focus");
        letterbox.classList.add("blank");
        letterbox.innerHTML = "";
        --current_col;
    } else if (event.key === "Enter" && current_col === num_cols) {
        let word = "";
        for (let j = 0; j < num_cols; ++j) {
            word += getLetterBox(current_row, j).innerHTML.toLowerCase();
        }
        ws.send(word);
    }
};

ws.onmessage = (event) => {
    let message = event.data;
    if (!message) {
        return;
    }
    if (message[0] === '!') {
        setTimeout(() => {
            window.alert("The answer was '" + message.substring(1) + "'");
        }, 300 * num_cols);
        finished = true;
        return;
    }
    for (let j = 0; j < num_cols; ++j) {
        let letterbox = getLetterBox(current_row, j);
        setTimeout(() => {
            letterbox.classList.remove("focus");
            if (message[j] === '.') {
                letterbox.classList.add("absent");
            } else if (message[j] === 'y') {
                letterbox.classList.add("present");
            } else if (message[j] === 'g') {
                letterbox.classList.add("correct");
            }
        }, 300 * j);
    }
    ++current_row;
    current_col = 0;
    if (message === 'g'.repeat(num_cols)) {
        setTimeout(() => {
            window.alert("You won!");
        }, 300 * num_cols);
        finished = true;
    }
}
