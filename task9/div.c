int divide(int operand1, int operand2) {
    if (operand2 != 0) {
        return operand1 / operand2;
    }

    return 0;    
}

const char* get_plugin_menu_text() {
    return "Divide";
}

int plugin_operation(int operand1, int operand2) {
    return divide(operand1, operand2);
}