int substract(int operand1, int operand2) {
    return operand1 - operand2;
}

const char* get_plugin_menu_text() {
    return "Substract";
}

int plugin_operation(int operand1, int operand2) {
    return substract(operand1, operand2);
}