int multiple(int operand1, int operand2) {
    return operand1 * operand2;
}

const char* get_plugin_menu_text() {
    return "Multiple";
}

int plugin_operation(int operand1, int operand2) {
    return multiple(operand1, operand2);
}