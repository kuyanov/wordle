const num_rows = 6;
const num_cols = 5;

let current_row = 0;
let current_col = 0;
let keyboard_disabled = true;
let key_rows = ["qwertyuiop", "asdfghjkl", "zxcvbnm"];
let params = new URLSearchParams(window.location.search);
let mode = params.get("mode") || "random"
let id = Math.random().toString(36).substring(2, 10);
let ws = null;

let field = document.getElementById("field");
let keyboard = document.getElementById("keyboard");

function ignore(event) {
    event.preventDefault();
}

function isLetter(str) {
    return str.length === 1 && ((str >= 'a' && str <= 'z') || (str >= 'A' && str <= 'Z'))
}

function initField() {
    field.innerHTML = "";
    for (let i = 0; i < num_rows; ++i) {
        let row = document.createElement("div");
        row.classList.add("field-row");
        for (let j = 0; j < num_cols; ++j) {
            let letterbox = document.createElement("div");
            letterbox.classList.add("letterbox");
            letterbox.classList.add("blank");
            row.appendChild(letterbox);
        }
        field.appendChild(row);
    }
}

function initKeyboard() {
    keyboard.innerHTML = "";
    let backspace_key = document.createElement("button");
    backspace_key.classList.add("key");
    backspace_key.classList.add("control");
    backspace_key.innerHTML = "<i class=\"fas fa-backspace\"></i>";
    backspace_key.onclick = onBackspace;
    backspace_key.onkeydown = ignore;
    let enter_key = document.createElement("button");
    enter_key.classList.add("key");
    enter_key.classList.add("control");
    enter_key.innerHTML = "<i class=\"fas fa-arrow-right\"></i>";
    enter_key.onclick = onEnter;
    enter_key.onkeydown = ignore;
    for (let row_id = 0; row_id < key_rows.length; ++row_id) {
        let row = document.createElement("div");
        row.classList.add("keyboard-row");
        if (row_id === 2) {
            row.appendChild(backspace_key);
        }
        for (let c of key_rows[row_id]) {
            let key = document.createElement("button");
            key.classList.add("key");
            key.classList.add("blank");
            key.innerHTML = c;
            key.onclick = () => onLetter(c);
            key.onkeydown = ignore;
            row.appendChild(key);
        }
        if (row_id === 2) {
            row.appendChild(enter_key);
        }
        keyboard.appendChild(row);
    }
}

function getLetterBox(row, col) {
    return field.children[row].children[col];
}

function getKey(letter) {
    for (let row_id = 0; row_id < key_rows.length; ++row_id) {
        for (let col_id = 0; col_id < key_rows[row_id].length; ++col_id) {
            if (key_rows[row_id][col_id] === letter) {
                if (row_id === 2) ++col_id;
                return keyboard.children[row_id].children[col_id];
            }
        }
    }
}

function shakeKeysVertically() {
    for (let j = 0; j < num_cols; ++j) {
        let letterbox = getLetterBox(current_row, j);
        setTimeout(() => {
            letterbox.classList.add("animated-vertically");
            letterbox.classList.add("shaken-up");
        }, 300 * num_cols + 100 * j);
        setTimeout(() => {
            letterbox.classList.remove("shaken-up");
        }, 300 * num_cols + 100 * j + 200);
        setTimeout(() => {
            letterbox.classList.remove("animated-vertically");
        }, 300 * num_cols + 100 * j + 400);
    }
}

function shakeKeysHorizontally() {
    for (let j = 0; j < num_cols; ++j) {
        let letterbox = getLetterBox(current_row, j);
        setTimeout(() => {
            letterbox.classList.add("animated-horizontally");
            letterbox.classList.add("shaken-left");
        }, 0);
        setTimeout(() => {
            letterbox.classList.remove("shaken-left");
            letterbox.classList.add("shaken-right");
        }, 50);
        setTimeout(() => {
            letterbox.classList.remove("shaken-right");
            letterbox.classList.add("shaken-left");
        }, 100);
        setTimeout(() => {
            letterbox.classList.remove("shaken-left");
            letterbox.classList.add("shaken-right")
        }, 150);
        setTimeout(() => {
            letterbox.classList.remove("shaken-right");
        }, 200);
        setTimeout(() => {
            letterbox.classList.remove("animated-horizontally");
        }, 250);
    }
}

function onLetter(letter) {
    if (!keyboard_disabled && current_row < num_rows && current_col < num_cols) {
        let letterbox = getLetterBox(current_row, current_col);
        letterbox.classList.remove("blank");
        letterbox.classList.add("focus");
        letterbox.innerHTML = letter;
        ++current_col;
    }
}

function onBackspace() {
    if (!keyboard_disabled && current_row < num_rows && current_col > 0) {
        let letterbox = getLetterBox(current_row, current_col - 1);
        letterbox.classList.remove("focus");
        letterbox.classList.add("blank");
        letterbox.innerHTML = "";
        --current_col;
    }
}

function onEnter() {
    if (!keyboard_disabled && current_col === num_cols) {
        keyboard_disabled = true;
        let word = "";
        for (let j = 0; j < num_cols; ++j) {
            word += getLetterBox(current_row, j).innerHTML;
        }
        ws.send(word);
    }
}

function onMessage(message) {
    if (!message) {
        shakeKeysHorizontally();
        setTimeout(() => {
            keyboard_disabled = false;
        }, 250);
        return;
    }
    if (message[0] === '!') {
        setTimeout(() => {
            window.alert("The answer was '" + message.substring(1) + "'");
        }, 300 * num_cols);
        return;
    }
    for (let j = 0; j < num_cols; ++j) {
        let letterbox = getLetterBox(current_row, j);
        let key = getKey(letterbox.innerHTML);
        setTimeout(() => {
            letterbox.classList.remove("focus");
            if (message[j] === '.') {
                letterbox.classList.add("absent");
                if (key.className === "key blank") {
                    key.className = "key absent";
                }
            } else if (message[j] === 'y') {
                letterbox.classList.add("present");
                if (key.className === "key blank" || key.className === "key absent") {
                    key.className = "key present";
                }
            } else if (message[j] === 'g') {
                letterbox.classList.add("correct");
                key.className = "key correct";
            }
        }, 300 * j);
    }
    if (message === 'g'.repeat(num_cols)) {
        shakeKeysVertically();
    } else {
        setTimeout(() => {
            keyboard_disabled = false;
        }, 300 * num_cols);
    }
    ++current_row;
    current_col = 0;
}

function connect() {
    ws = new WebSocket(`ws://${location.hostname}:${location.port}/${mode}/${id}`);
    ws.onopen = () => {
        keyboard_disabled = false;
    }
    ws.onmessage = (event) => {
        onMessage(event.data);
    }
    ws.onerror = () => {
        ws.close();
    }
    ws.onclose = () => {
        keyboard_disabled = true;
        setTimeout(connect, 1000);
    }
}

initField();
initKeyboard();
connect();

window.onkeydown = (event) => {
    if (isLetter(event.key)) {
        onLetter(event.key);
    } else if (event.key === "Backspace") {
        onBackspace();
    } else if (event.key === "Enter") {
        onEnter();
    }
};
