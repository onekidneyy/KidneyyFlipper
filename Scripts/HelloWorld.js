let keyboard = require("keyboard");

keyboard.setHeader("Enter you name");

let name = keyboard.text(20, "", true);

if (name !== undefined) {
    print("Hello " + name);
}
else {
    print("Hello World");
}










