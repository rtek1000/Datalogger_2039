/*
 * language.h
 *
 *  Created on: 26 de out de 2020
 *      Author: r
 */

#ifndef INC_LANGUAGE_H_
#define INC_LANGUAGE_H_

const int size_menu_item = 27;

const char main_menu_item[3][8][28] = { { { "E: Export" }, { "C: Chart" }, {
		"P: Password" }, { "S: Sensors" }, { "T: Clock" }, { "V: Memory Test" },
		{ "L: Language/Idiom & Setting" } },

{ { "E: Exportar" }, { "G: Grafico" }, { "P: Contrasena" }, { "S: Sensores" }, {
		"T: Reloj" }, { "V: Prueba de Memoria" }, {
		"L: Idioma/Language & Ajuste" } }, {

{ "E: Exportar" }, { "G: Grafico" }, { "P: Senha" }, { "S: Sensores" }, {
		"T: Relogio" }, { "V: Testar Memoria" },
		{ "L: Idioma/Language & Config" } } };

const char txtDay[3][8][4] = { { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri",
		"Sat", "BAT" },
		{ "Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab", "BAT" }, { "Dom",
				"Seg", "Ter", "Qua", "Qui", "Sex", "Sab", "BAT" } };

const char config_clock_title[3][20] = { "SETTING THE CLOCK",
		"AJUSTE DEL RELOJ", "CONFIGURAR RELOGIO" };

const char config_clock_date[3][6] = { "Date", "Fecha", "Data" };
const char config_clock_date2[3][8] = { "Weekday", "Semana", "Semana" };
const char config_clock_time[3][6] = { "Hour", "Hora", "Hora" };
const char config_clock_tab[3][15] = { "toggles fields", "alterna campos",
		"alterna campos" };
const char config_clock_enter[3][27] = { "ENTER: Confirm, ESC: Exit",
		"ENTER: Conf., ESC: Salir", "ENTER: Confirm., ESC: Sair" };

const char progress_bar_status[3][8] = { "Export.", "Export.", "Export." };
const char progress_bar_status1[3][9] = { "Check.", "Comprob.", "Verif." };

const char progress_bar_status2[3][11] =
		{ "Finished", "Terminado", "Finalizado" };
const char progress_bar_status3[3][9] =
		{ "Scanning", "Buscando", "Buscando" };

const char main_u_disk[3][10] = { "USB Disk", "Pendrive", "Pen Drive" };
const char main_keyboard[3][9] = { "Keyboard", "Teclado", "Teclado" };

const char usb_export_title[3][15] = { "Export data", "Exportar datos",
		"Exportar dados" };
const char usb_export_channel[3][9] = { "Channels", "Canales", "Canais" };
const char usb_export_note1[3][21] = { "A: all, N: none",
		"T: todos, N: ninguno", "T: todos, N: nenhum" };
const char usb_export_note2[3][19] = { "F: file type",
		"F: tipo de archivo", "F: tipo de arquivo" };
const char usb_export_label1[3][20] = { "Waiting for command",
		"Esperando el mando", "Aguardando comando" };
const char usb_export_label2[3][13] = { "Unplugged", "Desenchufado",
		"Desconectado" };
const char usb_export_label3[3][13] = { "Updated:", "Actualizado:",
		"Atualizado:" };
const char usb_export_error[3][6] = { "ERROR", "ERROR", "ERRO" };
const char usb_export_error99[3][13] = { "Disconnected", "Desconectado",
		"Sesconectado" };
const char usb_export_label4[3][27] = { "ENTER: Confirm, ESC: Exit",
		"ENTER: Conf., ESC: Salir", "ENTER: Confirm., ESC: Sair" };

const char memTestStatusStr[6][5] = { "----", "Test", "Falh", "Pass", "Bloq",
		"----" };

const char sensors_monitor_title[3][20] = { "Monitor sensors",
		"Monitorear sensores", "Monitorar sensores" };
const char sensors_monitor_offset_title[3][14] = { "Sensor Offset",
		"Sensor Offset", "Sensor Offset" };
const char sensors_monitor_desc[3][16] = { "CH   PRES  ON", "CAN  PRES  ACT",
		"CAN  PRES  LIG" };
const char sensors_monitor_yes[3][4] = { "YES", "SI", "SIM" };
const char sensors_monitor_no[3][4] = { "NO", "NO", "NAO" };
const char sensors_monitor_bat[3][8] = { "Bat", "Bat", "Bat" };
const char sensors_monitor_charg[3][11] = { "Chg", "Fnt", "Fnt" };
const char sensors_monitor_ang[3][7] = { "Angle", "Angulo", "Angulo" };

const char sensors_config_ch_title[3][18] = { "Configure Channel",
		"Configurar Canal", "Configurar Canal" };
const char sensors_config_rec[3][17] = { "Record data:", "Registrar datos:",
		"Registrar dados:" };
const char sensors_config_start[3][10] = { "Start:", "Comienzo:", "Inicio:" };
const char sensors_config_stop[3][6] = { "Stop:", "Fin:", "Fim:" };
const char sensors_config_label[3][28] = { "Label: (Max. 56 chars.)",
		"Etiqueta: (Max. 56 caract.)", "Etiqueta: (Max. 56 caract.)" };
const char sensors_config_inac[3][13] = { "  Stopped   ", "Interrumpido",
		"  Parado   " };
const char sensors_config_inac2[3][13] = { "[ Inactive ]", "[ Inactivo ]",
		"[ Inativo ]" };
const char sensors_config_act[3][13] = { " Recording  ", " Grabacion  ",
		" Gravando  " };
const char sensors_config_act2[3][13] = { " [ Active ] ", " [ Activo ] ",
		" [ Ativo ] " };

const char config_ch_erase_label1[3][8] = { "Channel", "Canal", "Canal" };
const char config_ch_erase_label2[3][15] = { "Clear memory", "Borrar memoria",
		"Limpar memoria" };

const char config_ch_confirm_label1[3][17] = { "Change tag", "Cambiar etiqueta",
		"Alterar etiqueta" };
const char config_ch_confirm_label2[3][24] = { "Replace label?",
		"Reemplazar la etiqueta?", "Substituir etiqueta?" };
const char config_ch_confirm_label3[3][18] = { "Change records",
		"Cambiar registros", "Alterar registros" };
const char config_ch_confirm_label4[3][21] = { "Stop logging?",
		"Detener el registro?", "Parar registros?" };
const char config_ch_confirm_label5[3][21] = { "Start logging?",
		"Empezar el registro?", "Comecar registros?" };
const char config_ch_confirm_label6[3][24] = { "(Block tag?)",
		"(Bloquear la etiqueta?)", "(Bloquear etiqueta?)" };
const char config_ch_confirm_label7[3][8] = { "< YES >", "< SI >", "< SIM >" };
const char config_ch_confirm_label8[3][8] = { "  NO  ", "  NO  ", "  NAO  " };
const char config_ch_confirm_label9[3][26] = { "(Yes: replace data)",
		"(Si: reemplace los datos)", "(Sim: substituir dados)" };
const char config_ch_confirm_label10[3][28] = { "(No: back without changing)",
		"(No: volver sin cambiar)", "(Nao: voltar sem alterar)" };
const char config_ch_confirm_label11[3][8] = { "  YES  ", "  SI  ", "  SIM  " };
const char config_ch_confirm_label12[3][8] = { "< NO >", "< NO >", "< NAO >" };
const char config_ch_confirm_label13[3][29] = { "ENTER: confirm, ESC: exit",
		"ENTER: confirmar, ESC: salir", "ENTER: confirmar, ESC: sair" };
const char config_ch_confirm_label14[3][28] = { "Warning: Export data",
		"Advertencia: exportar datos", "Aviso: Exportar dados" };
const char config_ch_confirm_label15[3][21] = { "Data not exported",
		"Datos no exportados", "Dados nao exportados" };
const char config_ch_confirm_label16[3][17] = { "ENTER: confirm",
		"ENTER: confirmar", "ENTER: confirmar" };

const char config_language_label1[3][25] = { "Language/Idiom & Setting",
		"Idioma/Language & Ajuste", "Idioma/Language & Config" };
const char config_language_label2[3][8] = { "-Date:", "-Fecha:", "-Data:" };
const char config_language_label3[3][12] = { "-Separator:", "-Separador:",
		"-Separador:" };
const char config_language_label4[3][9] = { "-Degree:", "-Grados:", "-Graus:" };
const char config_language_label5[3][29] = { "(ENTER: Confirm, ESC: Exit)",
		"(ENTER: Conf., ESC: Salir)", "(ENTER: Confirm., ESC: Sair)" };
const char config_language_label6[3][9] = { "mm-dd-yy", "mm-dd-aa", "mm-dd-aa" };
const char config_language_label7[3][9] = { "dd/mm/yy", "dd/mm/aa", "dd/mm/aa" };

const char password_title[3][19] = { "Access control", "Controle de acceso",
		"Controle de acesso" };
const char password_label1[3][23] = { "Type the password:",
		"Escriba la contrasena:", "Digite a Senha:" };
const char password_label2[3][23] = { "Type the password:",
		"Escriba la contrasena:", "Digite a Senha:" };

const char drawInfo_Password_label1[3][25] = { "Incorrect password...",
		"Contrasena incorrecta...", "Senha incorreta..." };

const char password_config_title[3][20] = { "Set password",
		"Configurar la clave", "Configurar senha" };
const char password_config_label1[3][23] = { "Updated password",
		"Contrasena actualizada", "Senha atualizada" };
const char password_config_label2[3][22] = { "Incorrect password",
		"Contrasena incorrecta", "Senha incorreta" };
const char password_config_label3[3][19] = { "Current password:",
		"Contrasena actual:", "Senha atual:" };
const char password_config_label4[3][18] = { "New password:",
		"Nueva contrasena:", "Senha nova:" };

const char memory_test_title[3][18] = { "Memory test", "Prueba de memoria",
		"Teste da memoria" };
const char memory_test_label1[3][10] = { "Channels:", "Canales:", "Canais:" };
const char memory_test_label2[3][23] = { "(A: all, N: none)",
		"(T: todos, N: ninguno)", "(T: todos,  N: nenhum)" };
const char memory_test_label3[3][29] = { "ENTER: Start | ESC: Back",
		"ENTER: Iniciar | ESC: Atras", "ENTER: Iniciar | ESC: Voltar" };
const char memory_test_status1[3][29] = { "Status: Waiting for command ",
		"Estado: Esperando el mando  ", "Status: Aguardando comando  " };
const char memory_test_status2[3][29] = { "Status: Select channel      ",
		"Estado: Seleccionar canal   ", "Status: Selecionar canal    " };

#endif /* INC_LANGUAGE_H_ */
