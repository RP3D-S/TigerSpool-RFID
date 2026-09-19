#include "i18n.h"
#include <Preferences.h>

// One row per string, all languages side by side, in the order of enum Lang:
//   EN, FR, DE, ES, IT, PL, PT-BR, PT-PT
//
// These strings are written WITHOUT diacritics, and that is not an oversight.
// The compiled Montserrat covers 0x20-0x7F plus degree and bullet - ASCII, not
// Latin-1 - so an accent draws as a blank box and LVGL logs nothing. Restoring
// them needs a generated Latin subset font first, and Polish needs Latin
// Extended-A beyond that. scripts/check-ui-fonts.py enforces this, and widens
// by itself once such a font exists.
//
// The screen is 240 px wide. A string that is half again as long in German as
// in English gets cut off, so translations are kept short rather than literal.
struct Row { const char* s[LANG_N]; };

static const Row STR[S_COUNT] = {
/* S_TOUCH_SLOT     */ {{ "tap a slot", "touchez un emplacement", "Slot antippen", "toca una ranura", "tocca uno slot", "dotknij gniazda", "toque um slot", "toque num slot", "点击槽位" }},
/* S_BRING_TAG      */ {{ "Hold the spool", "Approchez la bobine", "Spule anhalten", "Acerca la bobina", "Avvicina la bobina", "Przyloz szpule", "Aproxime a bobina", "Aproxime a bobine", "将料盘" }},
/* S_TO_READER      */ {{ "against the box", "contre le boîtier", "an das Gerat", "a la caja", "alla scatola", "do urzadzenia", "da caixa", "da caixa", "靠近设备" }},
/* S_CANCEL         */ {{ "Cancel", "Annuler", "Abbrechen", "Cancelar", "Annulla", "Anuluj", "Cancelar", "Cancelar", "取消" }},
/* S_SEND_TO        */ {{ "Send to %s?", "Envoyer vers %s ?", "An %s senden?", "Enviar a %s?", "Inviare a %s?", "Wyslac do %s?", "Enviar para %s?", "Enviar para %s?", "发送到 %s?" }},
/* S_NOZZLE         */ {{ "Nozzle", "Buse", "Duse", "Boquilla", "Ugello", "Dysza", "Bico", "Bico", "喷嘴" }},
/* S_BED            */ {{ "Bed", "Plateau", "Bett", "Cama", "Piano", "Stol", "Mesa", "Cama", "热床" }},
/* S_OK             */ {{ "OK", "OK", "OK", "OK", "OK", "OK", "OK", "OK", "确定" }},
/* S_ERR            */ {{ "Error", "Erreur", "Fehler", "Error", "Errore", "Blad", "Erro", "Erro", "错误" }},
/* S_TAP_BACK       */ {{ "tap to continue", "touchez pour continuer", "zum Fortfahren tippen", "toca para continuar", "tocca per continuare", "dotknij, aby kontynuowac", "toque para continuar", "toque para continuar", "点击继续" }},
/* S_CONNECTING     */ {{ "Connecting to", "Connexion à", "Verbinde mit", "Conectando a", "Connessione a", "Laczenie z", "Conectando a", "A ligar a", "正在连接" }},
/* S_WIFI_FAIL      */ {{ "Connection failed", "Échec de connexion", "Verbindung fehlgeschlagen", "Fallo de conexion", "Connessione fallita", "Blad polaczenia", "Falha na conexao", "Falha na ligacao", "连接失败" }},
/* S_NO_NETWORK     */ {{ "No network set up", "Aucun réseau configuré", "Kein Netzwerk eingerichtet", "Sin red configurada", "Nessuna rete configurata", "Brak skonfigurowanej sieci", "Nenhuma rede configurada", "Sem rede configurada", "未设置网络" }},
/* S_CONFIG_HINT    */ {{ "Set up Wi-Fi first", "Configurez le Wi-Fi", "Zuerst WLAN einrichten", "Configura el Wi-Fi", "Configura il Wi-Fi", "Najpierw skonfiguruj Wi-Fi", "Configure o Wi-Fi", "Configure o Wi-Fi", "请先设置 Wi-Fi" }},
/* S_UPDATED        */ {{ "%s updated", "%s mis à jour", "%s aktualisiert", "%s actualizado", "%s aggiornato", "%s zaktualizowano", "%s atualizado", "%s atualizado", "%s 已更新" }},
/* S_PRINTER_OFF    */ {{ "Printer unreachable", "Imprimante injoignable", "Drucker nicht erreichbar", "Impresora inaccesible", "Stampante irraggiungibile", "Drukarka niedostepna", "Impressora inacessivel", "Impressora inacessivel", "无法连接打印机" }},
/* S_SEND_FAIL      */ {{ "Could not send", "Envoi impossible", "Senden fehlgeschlagen", "No se pudo enviar", "Invio non riuscito", "Nie mozna wyslac", "Nao foi possivel enviar", "Nao foi possivel enviar", "发送失败" }},
/* S_HOLDER         */ {{ "Ext.", "Ext.", "Ext.", "Ext.", "Est.", "Zew.", "Ext.", "Ext.", "外置" }},
/* S_READ_UNSTABLE  */ {{ "Move the spool closer", "Rapprochez la bobine", "Spule naher halten", "Acerca mas la bobina", "Avvicina di piu la bobina", "Przysun szpule blizej", "Aproxime mais a bobina", "Aproxime mais a bobine", "请将料盘靠近" }},
/* S_BLANK_TAG      */ {{ "Tag is empty", "Étiquette vierge", "Tag ist leer", "Etiqueta vacia", "Tag vuoto", "Pusty tag", "Etiqueta vazia", "Etiqueta vazia", "标签为空" }},
/* S_PRINTER        */ {{ "Printers", "Imprimantes", "Drucker", "Impresoras", "Stampanti", "Drukarki", "Impressoras", "Impressoras", "打印机" }},
/* S_NO_PRINTERS    */ {{ "No printers yet", "Aucune imprimante", "Noch keine Drucker", "Sin impresoras", "Nessuna stampante", "Brak drukarek", "Nenhuma impressora", "Nenhuma impressora", "暂无打印机" }},
/* S_TT_LINKED      */ {{ "Account linked", "Compte connecté", "Konto verbunden", "Cuenta vinculada", "Account collegato", "Konto polaczone", "Conta vinculada", "Conta ligada", "账户已绑定" }},
/* S_ADD_WEB        */ {{ "Add them in Tiger Studio", "Ajoutez-les dans Tiger Studio", "In Tiger Studio hinzufugen", "Anadelas en Tiger Studio", "Aggiungile in Tiger Studio", "Dodaj je w Tiger Studio", "Adicione no Tiger Studio", "Adicione no Tiger Studio", "请在 Tiger Studio 中添加" }},
/* S_CONFIG_WEB     */ {{ "Settings:", "Réglages :", "Einstellungen:", "Ajustes:", "Impostazioni:", "Ustawienia:", "Ajustes:", "Definicoes:", "设置:" }},
/* S_SETTINGS        */ {{ "Settings", "Réglages", "Einstellungen", "Ajustes", "Impostazioni", "Ustawienia", "Ajustes", "Definicoes", "设置" }},
/* S_AP_TITLE       */ {{ "Wi-Fi setup", "Configuration Wi-Fi", "WLAN einrichten", "Configurar Wi-Fi", "Configura Wi-Fi", "Konfiguracja Wi-Fi", "Configurar Wi-Fi", "Configurar Wi-Fi", "Wi-Fi 设置" }},
/* S_CHANGE_NETWORK  */ {{ "Change network", "Changer de réseau", "Netzwerk wechseln", "Cambiar de red", "Cambia rete", "Zmien siec", "Trocar de rede", "Mudar de rede", "更换网络" }},
/* S_AP_JOIN         */ {{ "Scan the QR code", "Scanner le QR Code", "QR-Code scannen", "Escanea el código QR", "Inquadra il codice QR", "Zeskanuj kod QR", "Escaneie o QR Code", "Digitalize o código QR", "扫描二维码" }},
/* S_OR_JOIN         */ {{ "Or join Wi-Fi", "Ou rejoignez le Wi-Fi", "Oder WLAN beitreten", "O conecta al Wi-Fi", "O connettiti al Wi-Fi", "Lub polacz z Wi-Fi", "Ou conecte ao Wi-Fi", "Ou ligue ao Wi-Fi", "或连接 Wi-Fi" }},
/* S_OR_OPEN        */ {{ "Or open this address", "Ou ouvrez cette adresse", "Oder diese Adresse offnen", "O abre esta direccion", "O apri questo indirizzo", "Lub otworz ten adres", "Ou abra este endereco", "Ou abra este endereco", "或打开此地址" }},
/* S_AP_OPEN        */ {{ "Then open", "Puis ouvrez", "Dann offnen", "Luego abre", "Poi apri", "Nastepnie otworz", "Depois abra", "Depois abra", "然后打开" }},
/* S_AP_CHOOSE      */ {{ "and pick your network", "et choisissez votre réseau", "und Netzwerk wahlen", "y elige tu red", "e scegli la rete", "i wybierz siec", "e escolha sua rede", "e escolha a sua rede", "并选择您的网络" }},
/* S_TT_IMPORTING   */ {{ "Importing printers", "Import des imprimantes", "Drucker werden importiert", "Importando impresoras", "Importazione stampanti", "Importowanie drukarek", "Importando impressoras", "A importar impressoras", "正在导入打印机" }},
/* S_TT_ACCOUNT     */ {{ "Account", "Compte", "Konto", "Cuenta", "Account", "Konto", "Conta", "Conta", "账户" }},
/* S_ONLINE         */ {{ "online", "en ligne", "online", "en linea", "online", "online", "online", "online", "在线" }},
/* S_OFFLINE        */ {{ "offline", "hors ligne", "offline", "sin conexion", "offline", "offline", "offline", "offline", "离线" }},
/* S_BACK           */ {{ "Back", "Retour", "Zuruck", "Atras", "Indietro", "Wstecz", "Voltar", "Voltar", "返回" }},
/* S_FIND_PRINTERS  */ {{ "Looking for printers", "Recherche d'imprimantes", "Suche nach Druckern", "Buscando impresoras", "Ricerca stampanti", "Szukam drukarek", "Procurando impressoras", "A procurar impressoras", "正在查找打印机" }},
/* S_NO_ONLINE      */ {{ "No printer reachable", "Aucune imprimante joignable", "Kein Drucker erreichbar", "Ninguna impresora accesible", "Nessuna stampante raggiungibile", "Zadna drukarka niedostepna", "Nenhuma impressora acessivel", "Nenhuma impressora acessivel", "没有可连接的打印机" }},
/* S_WIFI_BAD_PASSWORD */ {{ "Couldn't join. The password may be wrong.", "Échec. Le mot de passe est peut-être faux.", "Fehlgeschlagen. Passwort evtl. falsch.", "Fallo. La contrasena puede estar mal.", "Fallito. La password potrebbe essere errata.", "Nie udalo sie. Haslo moze byc bledne.", "Falhou. A senha pode estar errada.", "Falhou. A palavra-passe pode estar errada.", "无法连接, 密码可能有误。" }},
/* S_ACCOUNT_WHY    */ {{ "Your printers are already in your TigerTag account. Link it and they arrive by themselves.", "Vos imprimantes sont déjà dans votre compte TigerTag. Connectez-le et elles arrivent seules.", "Ihre Drucker sind schon in Ihrem TigerTag-Konto. Verbinden und sie erscheinen von selbst.", "Tus impresoras ya estan en tu cuenta TigerTag. Vinculala y apareceran solas.", "Le tue stampanti sono gia nel tuo account TigerTag. Collegalo e arrivano da sole.", "Twoje drukarki sa juz na koncie TigerTag. Polacz je, a pojawia sie same.", "Suas impressoras ja estao na sua conta TigerTag. Vincule e elas chegam sozinhas.", "As suas impressoras ja estao na sua conta TigerTag. Ligue-a e chegam sozinhas.", "您的打印机已在 TigerTag 账户中。绑定后将自动导入。" }},
/* S_LINK_ACCOUNT   */ {{ "Link my account", "Connecter mon compte", "Konto verbinden", "Vincular mi cuenta", "Collega il mio account", "Polacz moje konto", "Vincular minha conta", "Ligar a minha conta", "绑定我的账户" }},
/* S_SIGN_IN         */ {{ "Log in to\nyour account", "Connectez\nvotre compte", "Melden Sie\nsich an", "Accede a\ntu cuenta", "Accedi al\ntuo account", "Zaloguj sie\nna konto", "Acesse\nsua conta", "Aceda a\nsua conta", "登录\n您的账户" }},
/* S_WAITING         */ {{ "Waiting...", "Patientez...", "Bitte warten...", "Esperando...", "Attendere...", "Czekaj...", "Aguarde...", "Aguarde...", "等待中..." }},
/* S_WITH_EMAIL      */ {{ "Mail & Password", "Mail & mot de passe", "Mail & Passwort", "Mail y contrasena", "Mail e password", "Mail i haslo", "Mail e senha", "Mail e palavra-passe", "邮箱和密码" }},
/* S_WITH_GOOGLE     */ {{ "Continue with Google", "Continuer avec Google", "Mit Google fortfahren", "Continuar con Google", "Continua con Google", "Kontynuuj z Google", "Continuar com o Google", "Continuar com o Google", "使用 Google 继续" }},
/* S_COLOUR_ADAPTED */ {{ "Colour adapted to the printer's palette", "Couleur adaptée à la palette de l'imprimante", "Farbe an die Druckerpalette angepasst", "Color adaptado a la paleta de la impresora", "Colore adattato alla palette della stampante", "Kolor dopasowany do palety drukarki", "Cor adaptada a paleta da impressora", "Cor adaptada a paleta da impressora", "颜色已适配打印机色板" }},
/* S_SCREEN         */ {{ "Screen", "Écran", "Anzeige", "Pantalla", "Schermo", "Ekran", "Tela", "Ecra", "屏幕" }},
/* S_LANGUAGE       */ {{ "Language", "Langue", "Sprache", "Idioma", "Lingua", "Jezyk", "Idioma", "Idioma", "语言" }},
/* S_UPDATE         */ {{ "Update", "Mise à jour", "Update", "Actualizar", "Aggiorna", "Aktualizacja", "Atualizar", "Atualizar", "更新" }},
/* S_RESTART        */ {{ "Restart", "Redémarrer", "Neustart", "Reiniciar", "Riavvia", "Uruchom ponownie", "Reiniciar", "Reiniciar", "重启" }},
/* S_FACTORY        */ {{ "Factory reset", "Réinit. usine", "Werksreset", "Rest. de fabrica", "Ripristino", "Reset fabryczny", "Restauracao", "Reposicao", "恢复出厂设置" }},
/* S_SIGN_OUT       */ {{ "Sign out", "Déconnexion", "Abmelden", "Cerrar sesion", "Esci", "Wyloguj", "Sair", "Terminar sessao", "退出登录" }},
/* S_BRIGHTNESS     */ {{ "Brightness", "Luminosité", "Helligkeit", "Brillo", "Luminosita", "Jasnosc", "Brilho", "Brilho", "亮度" }},
/* S_SLEEP_AFTER    */ {{ "Sleep after", "Veille après", "Ruhe nach", "Reposo tras", "Standby dopo", "Uspij po", "Suspender apos", "Suspender apos", "休眠时间" }},
/* S_NEVER          */ {{ "Never", "Jamais", "Nie", "Nunca", "Mai", "Nigdy", "Nunca", "Nunca", "从不" }},
/* S_INSTALLED      */ {{ "Version", "Version", "Version", "Version", "Versione", "Wersja", "Versao", "Versao", "版本" }},
/* S_OTA_OFF        */ {{ "Over-the-air updates are not enabled on this build.", "Les mises à jour par le réseau ne sont pas activées.", "Updates uber Funk sind in diesem Build nicht aktiv.", "Las actualizaciones por red no estan activas.", "Gli aggiornamenti via rete non sono attivi.", "Aktualizacje przez siec nie sa wlaczone.", "As atualizacoes pela rede nao estao ativas.", "As atualizacoes pela rede nao estao ativas.", "此版本未启用在线更新。" }},
/* S_RESTART_Q      */ {{ "Restart the TigerSpool?", "Redémarrer la TigerSpool ?", "TigerSpool neu starten?", "Reiniciar la TigerSpool?", "Riavviare la TigerSpool?", "Uruchomic ponownie TigerSpool?", "Reiniciar o TigerSpool?", "Reiniciar o TigerSpool?", "重启 TigerSpool?" }},
/* S_RESTORE        */ {{ "Restore", "Restaurer", "Wiederherstellen", "Restaurar", "Ripristina", "Przywroc", "Restaurar", "Restaurar", "恢复" }},
/* S_CONFIRM        */ {{ "Confirm", "Valider", "Bestatigen", "Confirmar", "Conferma", "Potwierdz", "Confirmar", "Confirmar", "确认" }},
/* S_FACTORY_WARN   */ {{ "Restore factory settings?", "Êtes-vous sûr de vouloir restaurer les paramètres d'usine ?", "Werkseinstellungen wiederherstellen?", "Restaurar los ajustes de fabrica?", "Ripristinare le impostazioni di fabbrica?", "Przywrocic ustawienia fabryczne?", "Restaurar as definicoes de fabrica?", "Restaurar as definicoes de fabrica?", "恢复出厂设置?" }},
/* S_HOLD_ERASE     */ {{ "Hold to erase", "Maintenir pour effacer", "Zum Loschen halten", "Manten para borrar", "Tieni per cancellare", "Przytrzymaj, aby usunac", "Segure para apagar", "Mantenha para apagar", "长按清除" }},
/* S_KEEP_HOLDING   */ {{ "Keep holding...", "Continuez...", "Weiter halten...", "Sigue pulsando...", "Continua a tenere...", "Trzymaj dalej...", "Continue segurando...", "Continue a premir...", "继续按住..." }},
/* S_CHECK_UPDATE   */ {{ "Check for updates", "Rechercher une mise à jour", "Nach Updates suchen", "Buscar actualizaciones", "Cerca aggiornamenti", "Sprawdz aktualizacje", "Procurar atualizacoes", "Procurar atualizacoes", "检查更新" }},
/* S_CHECKING       */ {{ "Checking...", "Recherche...", "Suche...", "Buscando...", "Ricerca...", "Sprawdzanie...", "Procurando...", "A procurar...", "检查中..." }},
/* S_UP_TO_DATE     */ {{ "Your TigerSpool is up to date", "Votre TigerSpool est à jour", "Ihr TigerSpool ist aktuell", "Tu TigerSpool esta actualizado", "Il tuo TigerSpool e aggiornato", "Twoj TigerSpool jest aktualny", "Seu TigerSpool esta atualizado", "O seu TigerSpool esta atualizado", "您的 TigerSpool 已是最新" }},
/* S_INSTALL        */ {{ "Install", "Installer", "Installieren", "Instalar", "Installa", "Zainstaluj", "Instalar", "Instalar", "安装" }},
/* S_DOWNLOADING    */ {{ "Downloading", "Téléchargement", "Wird geladen", "Descargando", "Download", "Pobieranie", "Baixando", "A transferir", "下载中" }},
/* S_DONT_UNPLUG    */ {{ "Do not unplug the box.", "Ne débranchez pas le boîtier.", "Gerat nicht trennen.", "No desconectes la caja.", "Non scollegare la scatola.", "Nie odlaczaj urzadzenia.", "Nao desconecte a caixa.", "Nao desligue a caixa.", "请勿断开设备电源。" }},
/* S_UPDATE_KEEPS   */ {{ "Wi-Fi, account and printers are kept.", "Wi-Fi, compte et imprimantes sont conservés.", "WLAN, Konto und Drucker bleiben erhalten.", "Wi-Fi, cuenta e impresoras se conservan.", "Wi-Fi, account e stampanti sono conservati.", "Wi-Fi, konto i drukarki zostaja zachowane.", "Wi-Fi, conta e impressoras sao mantidos.", "Wi-Fi, conta e impressoras sao mantidos.", "Wi-Fi、账户和打印机将保留。" }},
/* S_LATER          */ {{ "Later", "Plus tard", "Spater", "Mas tarde", "Piu tardi", "Pozniej", "Depois", "Mais tarde", "稍后" }},
/* S_SIGNAL         */ {{ "Signal", "Signal", "Signal", "Senal", "Segnale", "Sygnal", "Sinal", "Sinal", "信号" }},
/* S_AUTO           */ {{ "Auto", "Auto", "Auto", "Auto", "Auto", "Auto", "Auto", "Auto", "自动" }},
/* S_TAG_PRODUCT    */ {{ "ID Product", "ID Produit", "ID Produkt", "ID Producto", "ID Prodotto", "ID Produkt", "ID Produto", "ID Produto", "产品 ID" }},
/* S_TAG_TYPE       */ {{ "Type", "Type", "Typ", "Tipo", "Tipo", "Typ", "Tipo", "Tipo", "类型" }},
/* S_TAG_BRAND      */ {{ "Brand", "Marque", "Marke", "Marca", "Marca", "Marka", "Marca", "Marca", "品牌" }},
/* S_TAG_ASPECT     */ {{ "Aspect 1/2", "Aspect 1/2", "Aspekt 1/2", "Aspecto 1/2", "Aspetto 1/2", "Aspekt 1/2", "Aspecto 1/2", "Aspeto 1/2", "外观 1/2" }},
/* S_TAG_KIND       */ {{ "Kind", "Nature", "Art", "Tipo", "Genere", "Rodzaj", "Genero", "Genero", "种类" }},
/* S_TAG_PROTOCOL   */ {{ "Protocol", "Protocole", "Protokoll", "Protocolo", "Protocollo", "Protokol", "Protocolo", "Protocolo", "协议" }},
/* S_TAG_STAMP      */ {{ "TimeStamp", "Horodatage", "Zeitstempel", "Marca de tiempo", "Marca temporale", "Znacznik czasu", "Data e hora", "Data e hora", "时间戳" }},
/* S_TAG_DRY        */ {{ "Dry temp.", "Séchage temp.", "Trocknung Temp.", "Secado temp.", "Essiccazione temp.", "Suszenie temp.", "Secagem temp.", "Secagem temp.", "烘干温度" }},
/* S_TAG_QTY        */ {{ "Quantity", "Quantité", "Menge", "Cantidad", "Quantita", "Ilosc", "Quantidade", "Quantidade", "数量" }},
/* S_TAG_LEFT       */ {{ "Remaining", "Restant", "Verbleibend", "Restante", "Rimanente", "Pozostalo", "Restante", "Restante", "剩余" }},
/* S_TAG_COLOURS    */ {{ "Colours 2/3", "Couleurs 2/3", "Farben 2/3", "Colores 2/3", "Colori 2/3", "Kolory 2/3", "Cores 2/3", "Cores 2/3", "颜色 2/3" }},
/* S_TAG_TD         */ {{ "HueForge TD", "HueForge TD", "HueForge TD", "HueForge TD", "HueForge TD", "HueForge TD", "HueForge TD", "HueForge TD", "HueForge TD" }},
/* S_TAG_MESSAGE    */ {{ "Message", "Message", "Nachricht", "Mensaje", "Messaggio", "Wiadomosc", "Mensagem", "Mensagem", "留言" }},
/* S_TAG_SIG        */ {{ "Signature", "Signature", "Signatur", "Firma", "Firma", "Podpis", "Assinatura", "Assinatura", "签名" }},
/* S_SIG_VALID      */ {{ "genuine", "authentique", "echt", "autentica", "autentica", "autentyczny", "autentica", "autentica", "正品" }},
/* S_SIG_INVALID    */ {{ "does not match", "ne correspond pas", "passt nicht", "no coincide", "non corrisponde", "nie pasuje", "nao corresponde", "nao corresponde", "不匹配" }},
/* S_SIG_NONE       */ {{ "not signed", "non signée", "nicht signiert", "sin firmar", "non firmata", "niepodpisany", "sem assinatura", "sem assinatura", "未签名" }},
/* S_SIG_NOKEY      */ {{ "no public key", "pas de clé publique", "kein Schlussel", "sin clave publica", "nessuna chiave", "brak klucza", "sem chave publica", "sem chave publica", "无公钥" }},
/* S_SIG_UNREAD     */ {{ "not read", "non lue", "nicht gelesen", "no leida", "non letta", "nieodczytany", "nao lida", "nao lida", "未读取" }},
/* S_LINK_TRY       */ {{ "Connecting...", "Connexion...", "Verbinde...", "Conectando...", "Connessione...", "Laczenie...", "Conectando...", "A ligar...", "连接中..." }},
/* S_LINK_FETCH     */ {{ "Reading your account", "Récupération des données", "Konto wird gelesen", "Leyendo tu cuenta", "Lettura dell account", "Odczyt konta", "Lendo sua conta", "A ler a sua conta", "正在读取账户" }},
/* S_LINK_ATTEMPT   */ {{ "Attempt %d/%d", "Tentative %d/%d", "Versuch %d/%d", "Intento %d/%d", "Tentativo %d/%d", "Proba %d/%d", "Tentativa %d/%d", "Tentativa %d/%d", "第 %d/%d 次尝试" }},
/* S_LINK_FAIL      */ {{ "Connection failed", "Échec de connexion", "Verbindung fehlgeschlagen", "Fallo de conexion", "Connessione fallita", "Blad polaczenia", "Falha na conexao", "Falha na ligacao", "连接失败" }},
/* S_LF_SCAN        */ {{ "Scan the QR code", "Scanner le QR Code", "QR-Code scannen", "Escanea el codigo QR", "Scansiona il codice QR", "Zeskanuj kod QR", "Escaneie o QR code", "Digitalize o codigo QR", "扫描二维码" }},
/* S_CLOUD_ONLY     */ {{ "Working only with LAN Mode + Dev Mode", "Fonctionne uniquement en Mode LAN + Mode Dev", "Nur mit LAN-Modus + Dev-Modus", "Solo funciona con Modo LAN + Modo Dev", "Funziona solo con Modalita LAN + Modalita Dev", "Dziala tylko z trybem LAN + Dev", "Funciona apenas com Modo LAN + Modo Dev", "Funciona apenas com Modo LAN + Modo Dev", "仅支持局域网模式 + 开发者模式" }},
/* S_CLOUD_HOW      */ {{ "Scan for tutorial", "Scanner pour le tutoriel", "Fur die Anleitung scannen", "Escanea para el tutorial", "Scansiona per il tutorial", "Zeskanuj po samouczek", "Escaneie para o tutorial", "Digitalize para o tutorial", "扫码查看教程" }},
/* S_READER         */ {{ "NFC Tester", "NFC Tester", "NFC-Tester", "NFC Tester", "NFC Tester", "NFC Tester", "NFC Tester", "NFC Tester", "NFC 测试" }},
/* S_READ_MODE      */ {{ "Reader", "Lecteur", "Leser", "Lector", "Lettore", "Czytnik", "Leitor", "Leitor", "读取器" }},
/* S_READ_HINT      */ {{ "Read a spool", "Lire une bobine", "Spule lesen", "Leer una bobina", "Leggi una bobina", "Odczytaj szpule", "Ler uma bobina", "Ler uma bobine", "读取线盘" }},
/* S_READ_PUT_SPOOL */ {{ "Put a spool", "Posez une bobine", "Spule auflegen", "Coloca una bobina", "Appoggia una bobina", "Poloz szpule", "Coloque uma bobina", "Coloque uma bobine", "请放置线盘" }},
/* S_READ_ON_READER */ {{ "on the reader", "sur le lecteur", "auf den Leser", "en el lector", "sul lettore", "na czytniku", "no leitor", "no leitor", "在读取器上" }},
/* S_DRYING         */ {{ "Drying", "Séchage", "Trocknung", "Secado", "Essiccazione", "Suszenie", "Secagem", "Secagem", "烘干" }},
/* S_SENT_TO_PRINTER*/ {{ "Sent to the printer", "Envoyé à l'imprimante", "An den Drucker gesendet", "Enviado a la impresora", "Inviato alla stampante", "Wyslano do drukarki", "Enviado para a impressora", "Enviado para a impressora", "已发送到打印机" }},
/* S_INSERT_IN      */ {{ "Insert the spool in", "Insérez la bobine dans", "Spule einlegen in", "Inserta la bobina en", "Inserisci la bobina in", "Wloz szpule do", "Insira a bobina em", "Insira a bobine em", "请将线盘装入" }},
/* S_WRITE_MODE     */ {{ "Write", "Écriture", "Schreiben", "Escritura", "Scrittura", "Zapis", "Escrita", "Escrita", "写入" }},
/* S_SOON           */ {{ "Coming soon", "Bientôt disponible", "Demnächst", "Próximamente", "Prossimamente", "Wkrotce", "Em breve", "Em breve", "即将推出" }},
/* S_REMAINING      */ {{ "Remaining", "Restant", "Rest", "Restante", "Rimanente", "Pozostalo", "Restante", "Restante", "剩余" }},
/* S_READER_OK      */ {{ "Reader ready", "Lecteur prêt", "Leser bereit", "Lector listo", "Lettore pronto", "Czytnik gotowy", "Leitor pronto", "Leitor pronto", "读卡器就绪" }},
/* S_READER_NONE    */ {{ "Reader not found", "Lecteur introuvable", "Leser nicht gefunden", "Lector no encontrado", "Lettore non trovato", "Nie znaleziono czytnika", "Leitor nao encontrado", "Leitor nao encontrado", "未找到读卡器" }},
/* S_PRESENT_TAG    */ {{ "Hold a spool against the box", "Approchez une bobine du boîtier", "Spule an das Gerat halten", "Acerca una bobina a la caja", "Avvicina una bobina alla scatola", "Przyloz szpule do urzadzenia", "Aproxime uma bobina da caixa", "Aproxime uma bobina da caixa", "请将料盘靠近设备" }},
/* S_ORIENTATION   */ {{ "Orientation", "Orientation", "Ausrichtung", "Orientacion", "Orientamento", "Orientacja", "Orientacao", "Orientacao", "方向" }},
/* S_RESTARTING     */ {{ "Installed. Restarting...", "Installé. Redémarrage...", "Installiert. Neustart...", "Instalado. Reiniciando...", "Installato. Riavvio...", "Zainstalowano. Restart...", "Instalado. Reiniciando...", "Instalado. A reiniciar...", "已安装, 重启中..." }},
/* S_CHOOSE_PRINTERS */ {{ "Choose printers", "Vos imprimantes", "Drucker wahlen", "Elige impresoras", "Scegli stampanti", "Wybierz drukarki", "Suas impressoras", "Suas impressoras", "选择打印机" }},
/* S_SELECT_PRINTERS */ {{ "Select printers", "Choisir les imprimantes", "Drucker wahlen", "Elegir impresoras", "Scegli stampanti", "Wybierz drukarki", "Escolher impressoras", "Escolher impressoras", "选择打印机" }},
/* S_BATTERY        */ {{ "Battery", "Batterie", "Batterie", "Bateria", "Batteria", "Bateria", "Bateria", "Bateria", "电池" }},
/* S_BATT_VOLTAGE   */ {{ "Voltage", "Tension", "Spannung", "Tension", "Tensione", "Napiecie", "Tensao", "Tensao", "电压" }},
/* S_BATT_NOTE      */ {{ "Estimated from the voltage", "Estimation d'après la tension", "Aus der Spannung geschatzt", "Estimado por la tension", "Stimato dalla tensione", "Szacowane z napiecia", "Estimado pela tensao", "Estimado pela tensao", "根据电压估算" }},
/* S_BATT_CHARGING  */ {{ "Charging", "En charge", "Ladt", "Cargando", "In carica", "Ladowanie", "Carregando", "A carregar", "充电中" }},
/* S_BATT_CHARGE_NOTE */ {{ "Estimated, less exact while charging", "Estimation, moins exacte pendant la charge", "Schatzung, beim Laden ungenauer", "Estimado, menos exacto al cargar", "Stima, meno esatta durante la carica", "Szacowane, mniej dokladne przy ladowaniu", "Estimado, menos exato durante a carga", "Estimado, menos exato durante a carga", "估算值，充电时不够准确" }},
/* S_BATT_STATE     */ {{ "State", "État", "Status", "Estado", "Stato", "Stan", "Estado", "Estado", "状态" }},
/* S_BATT_ON_BATTERY */ {{ "On battery", "Sur batterie", "Akkubetrieb", "Con bateria", "A batteria", "Na baterii", "Na bateria", "Na bateria", "使用电池" }},
/* S_BATT_PLUG_IN   */ {{ "Plug it in", "Branchez le câble", "Kabel anschliessen", "Conecta el cable", "Collega il cavo", "Podlacz kabel", "Conecte o cabo", "Ligue o cabo", "请接通电源" }},
/* S_BATT_RUNTIME   */ {{ "Runtime left", "Autonomie", "Restlaufzeit", "Autonomia", "Autonomia", "Pozostaly czas", "Autonomia", "Autonomia", "剩余时间" }},
/* S_BATT_FULL_IN   */ {{ "Full in", "Charge pleine dans", "Voll in", "Completa en", "Carica in", "Pelna za", "Cheia em", "Cheia em", "充满还需" }},
/* S_UNIT_MIN       */ {{ "min", "min", "Min", "min", "min", "min", "min", "min", "分钟" }},
/* S_UNIT_HOUR      */ {{ "h", "h", "Std", "h", "h", "godz", "h", "h", "小时" }},
/* S_WAITING_NFC    */ {{ "Waiting NFC...", "En attente NFC...", "Warte auf NFC...", "Esperando NFC...", "In attesa NFC...", "Oczekiwanie na NFC...", "Aguardando NFC...", "A aguardar NFC...", "等待 NFC..." }},
/* S_TAG_DIAMETER   */ {{ "Diameter", "Diamètre", "Durchmesser", "Diametro", "Diametro", "Srednica", "Diametro", "Diametro", "直径" }},
/* S_TAG_DRY_TIME   */ {{ "Dry time", "Séchage durée", "Trocknungszeit", "Secado tiempo", "Essiccazione tempo", "Suszenie czas", "Secagem tempo", "Secagem tempo", "烘干时间" }},
/* ---------------------------------------------------------------------------
   The NFC Tester's field names, identical in every language.

   That screen is a bench instrument, not part of the product's interface: it
   dumps what came off the chip so it can be checked against a reader log, a
   datasheet or the TigerTag SDK - all of which are in English. Two people
   comparing the same spool in two countries have to be looking at the same
   words, and a translated "Weight Available" would have to be translated back
   before it could be compared with anything.

   They go through the table rather than being written into the screen because
   that is where drawn text lives; the rows are identical on purpose, and the
   guard that checks the table against the enum keeps them honest.
   --------------------------------------------------------------------------- */
/* S_NT_BRAND       */ {{ "Brand", "Brand", "Brand", "Brand", "Brand", "Brand", "Brand", "Brand", "Brand" }},
/* S_NT_TYPE        */ {{ "Type", "Type", "Type", "Type", "Type", "Type", "Type", "Type", "Type" }},
/* S_NT_MATERIAL    */ {{ "Material", "Material", "Material", "Material", "Material", "Material", "Material", "Material", "Material" }},
/* S_NT_MESSAGE     */ {{ "Message", "Message", "Message", "Message", "Message", "Message", "Message", "Message", "Message" }},
/* S_NT_ASPECT      */ {{ "Aspect 1/2", "Aspect 1/2", "Aspect 1/2", "Aspect 1/2", "Aspect 1/2", "Aspect 1/2", "Aspect 1/2", "Aspect 1/2", "Aspect 1/2" }},
/* S_NT_KIND        */ {{ "Kind", "Kind", "Kind", "Kind", "Kind", "Kind", "Kind", "Kind", "Kind" }},
/* S_NT_WTOTAL      */ {{ "Weight Total", "Weight Total", "Weight Total", "Weight Total", "Weight Total", "Weight Total", "Weight Total", "Weight Total", "Weight Total" }},
/* S_NT_WAVAIL      */ {{ "Weight Available", "Weight Available", "Weight Available", "Weight Available", "Weight Available", "Weight Available", "Weight Available", "Weight Available", "Weight Available" }},
/* S_NT_DIAM        */ {{ "Diam.", "Diam.", "Diam.", "Diam.", "Diam.", "Diam.", "Diam.", "Diam.", "Diam." }},
/* S_NT_NOZZLE      */ {{ "Nozzle Temp", "Nozzle Temp", "Nozzle Temp", "Nozzle Temp", "Nozzle Temp", "Nozzle Temp", "Nozzle Temp", "Nozzle Temp", "Nozzle Temp" }},
/* S_NT_BED         */ {{ "Bed Temp.", "Bed Temp.", "Bed Temp.", "Bed Temp.", "Bed Temp.", "Bed Temp.", "Bed Temp.", "Bed Temp.", "Bed Temp." }},
/* S_NT_DRY         */ {{ "Drying Temp. / Time", "Drying Temp. / Time", "Drying Temp. / Time", "Drying Temp. / Time", "Drying Temp. / Time", "Drying Temp. / Time", "Drying Temp. / Time", "Drying Temp. / Time", "Drying Temp. / Time" }},
/* S_NT_STAMP       */ {{ "Timestamp", "Timestamp", "Timestamp", "Timestamp", "Timestamp", "Timestamp", "Timestamp", "Timestamp", "Timestamp" }},
/* S_NT_DATE        */ {{ "Date", "Date", "Date", "Date", "Date", "Date", "Date", "Date", "Date" }},
/* S_NT_COLOR1      */ {{ "Color 1 (RGBA)", "Color 1 (RGBA)", "Color 1 (RGBA)", "Color 1 (RGBA)", "Color 1 (RGBA)", "Color 1 (RGBA)", "Color 1 (RGBA)", "Color 1 (RGBA)", "Color 1 (RGBA)" }},
/* S_NT_COLOR2      */ {{ "Color 2 (RGB)", "Color 2 (RGB)", "Color 2 (RGB)", "Color 2 (RGB)", "Color 2 (RGB)", "Color 2 (RGB)", "Color 2 (RGB)", "Color 2 (RGB)", "Color 2 (RGB)" }},
/* S_NT_COLOR3      */ {{ "Color 3 (RGB)", "Color 3 (RGB)", "Color 3 (RGB)", "Color 3 (RGB)", "Color 3 (RGB)", "Color 3 (RGB)", "Color 3 (RGB)", "Color 3 (RGB)", "Color 3 (RGB)" }},
/* S_NT_TD          */ {{ "TD HueForge", "TD HueForge", "TD HueForge", "TD HueForge", "TD HueForge", "TD HueForge", "TD HueForge", "TD HueForge", "TD HueForge" }},
/* S_NT_CERT        */ {{ "Certified", "Certified", "Certified", "Certified", "Certified", "Certified", "Certified", "Certified", "Certified" }},
/* S_NT_HEX         */ {{ "HEX Code", "HEX Code", "HEX Code", "HEX Code", "HEX Code", "HEX Code", "HEX Code", "HEX Code", "HEX Code" }},
/* S_NT_PAGE        */ {{ "Page", "Page", "Page", "Page", "Page", "Page", "Page", "Page", "Page" }},
/* S_NT_INVALID     */ {{ "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID", "INVALID" }},
/* S_NT_NONE        */ {{ "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE", "NONE" }},
/* S_NT_NOKEY       */ {{ "NO KEY", "NO KEY", "NO KEY", "NO KEY", "NO KEY", "NO KEY", "NO KEY", "NO KEY", "NO KEY" }},
/* S_LOAD           */ {{ "Load", "Charge", "Auslastung", "Carga", "Carico", "Obciążenie", "Carga", "Carga", "负载" }},
/* S_NO_ROOM        */ {{ "Not enough room", "Plus assez de place", "Kein Platz mehr", "No queda espacio", "Spazio esaurito", "Brak miejsca", "Sem espaço", "Sem espaço", "空间不足" }},
/* S_SIG_EXCELLENT  */ {{ "Excellent", "Excellent", "Ausgezeichnet", "Excelente", "Eccellente", "Doskonały", "Excelente", "Excelente", "极佳" }},
/* S_SIG_GOOD       */ {{ "Good", "Bon", "Gut", "Buena", "Buono", "Dobry", "Bom", "Bom", "良好" }},
/* S_SIG_FAIR       */ {{ "Fair", "Moyen", "Mittel", "Media", "Medio", "Średni", "Médio", "Médio", "一般" }},
/* S_SIG_WEAK       */ {{ "Weak", "Faible", "Schwach", "Débil", "Debole", "Słaby", "Fraco", "Fraco", "较弱" }},
/* S_CHANNEL        */ {{ "Channel", "Canal", "Kanal", "Canal", "Canale", "Kanał", "Canal", "Canal", "信道" }},
};

// A mismatch here is silent at runtime and reads as garbled text on screen, so
// let the compiler catch it instead.
static_assert(sizeof(STR) / sizeof(STR[0]) == S_COUNT,
              "i18n table and StrId enum are out of step");

// Each language written in itself: someone looking for Portugues should not
// have to recognise the English word "Portuguese" first.
static const char* const NAMES[LANG_N] = {
    "English", "Français", "Deutsch", "Español",
    "Italiano", "Polski", "Português (BR)", "Português (PT)", "中文"
};

namespace {
    Lang g_lang  = LANG_EN;
    bool g_chosen = false;
}

namespace i18n {

// The stored value is an index into enum Lang, so it only means anything to the
// enum that wrote it. The prototype ordered its languages PT, EN, ES, FR; this
// firmware orders them EN, FR, DE, ES, ... A value carried across without a
// marker silently selects a different language - a device set to French came up
// in Spanish, which is how this was found.
//
// Bump LANG_SCHEMA whenever enum Lang is reordered or has entries removed. An
// index from an older schema is discarded, the device falls back to English and
// asks once. Adding a language at the END does not need a bump.
static constexpr int LANG_SCHEMA = 2;

void begin() {
    Preferences p;
    p.begin("tigerspool", true);
    int schema = p.getInt("langVer", 0);
    int v      = p.getInt("lang", -1);
    p.end();

    if (schema != LANG_SCHEMA) {
        if (v >= 0) Serial.printf("[i18n] language index %d written by schema %d "
                                  "- discarding, asking again\n", v, schema);
        return;                       // stays English, stays unchosen
    }
    if (v >= 0 && v < LANG_N) { g_lang = (Lang)v; g_chosen = true; }
}

bool chosen()  { return g_chosen; }
Lang current() { return g_lang; }

void set(Lang l) {
    if (l >= LANG_N) return;
    g_lang = l; g_chosen = true;
    Preferences p;
    p.begin("tigerspool", false);
    p.putInt("lang", (int)l);
    p.putInt("langVer", LANG_SCHEMA);   // stamp what wrote it
    p.end();
}

// Falls back to English rather than returning null: a missing translation
// should show the English word, never crash a screen mid-draw.
const char* T(StrId id) {
    if (id >= S_COUNT) return "";
    const char* s = STR[id].s[g_lang];
    return (s && *s) ? s : STR[id].s[LANG_EN];
}

const char* name(Lang l) { return l < LANG_N ? NAMES[l] : NAMES[LANG_EN]; }

}  // namespace i18n
