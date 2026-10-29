"""@generated tools/gen_python.py -- не редактировать руками.

Типизированный слой над схемой протокола. Подмешивается в kolibri_core.Connection:
у клиента появляется по методу на операцию.

    with kb.Connection("api.oneme.ru", device_id=..., user_agent=ua) as c:
        c.handshake()
        r = c.auth_request(phone="+7...", type="START_AUTH", mode=fp)  # fp: bytes
        token = r.token

Имена полей сверяются со схемой (опечатка -> TypeError), bytes-поля сами уходят
как {"$bin": base64}, ответ -- объект с атрибутами-полями (плюс .raw с сырым dict).
"""

import base64
import types


def _build(op, spec, fields):
    out = {}
    for k, v in fields.items():
        if k not in spec:
            raise TypeError("%s: неизвестное поле %r; есть: %s"
                            % (op, k, ", ".join(spec) or "нет"))
        if v is None:
            continue
        if spec[k].rstrip("?") == "bytes" and isinstance(v, (bytes, bytearray)):
            v = {"$bin": base64.b64encode(bytes(v)).decode()}
        out[k] = v
    for k, t in spec.items():
        if not t.endswith("?") and k not in out:
            raise TypeError("%s: не хватает обязательного поля %r" % (op, k))
    return out


def _resp(d, names):
    obj = types.SimpleNamespace()
    for n in names:
        setattr(obj, n, None)
    if isinstance(d, dict):
        for k, v in d.items():
            if isinstance(k, str) and k.isidentifier():
                setattr(obj, k, v)
    obj.raw = d
    return obj


class Ops(object):
    """Методы-операции. Требует self.request(opcode, dict) -> dict."""


    def auth_request(self, **fields):
        """AUTH_REQUEST (17). Поля: phone:string, type:string, language:string?, mode:bytes?. Старт телефонной авторизации — сервер отправляет SMS с кодом."""
        _spec = {'phone': 'string', 'type': 'string', 'language': 'string?', 'mode': 'bytes?'}
        d = self.request(17, _build('AuthRequest', _spec, fields))
        return _resp(d, ['token'])

    def auth_verify(self, **fields):
        """AUTH (18). Поля: token:string, verifyCode:string, authTokenType:string. Проверка кода из SMS."""
        _spec = {'token': 'string', 'verifyCode': 'string', 'authTokenType': 'string'}
        d = self.request(18, _build('AuthVerify', _spec, fields))
        return _resp(d, ['tokenAttrs', 'profile', 'passwordChallenge', 'presetAvatars'])

    def login(self, **fields):
        """LOGIN (19). Поля: token:string, interactive:bool, exp:LoginExp?, chatCacheFingerprint:bytes?, presenceSync:i64?, chatsSync:i64?, contactsSync:i64?, callsSync:i64?, lastLogin:i64?, bannersSync:i64?, configHash:string?. Вход по login-токену: авторизует уже установленное соединение и отдаёт"""
        _spec = {'token': 'string', 'interactive': 'bool', 'exp': 'LoginExp?', 'chatCacheFingerprint': 'bytes?', 'presenceSync': 'i64?', 'chatsSync': 'i64?', 'contactsSync': 'i64?', 'callsSync': 'i64?', 'lastLogin': 'i64?', 'bannersSync': 'i64?', 'configHash': 'string?'}
        d = self.request(19, _build('Login', _spec, fields))
        return _resp(d, ['token', 'profile', 'chats', 'time', 'config', 'contacts', 'presence'])

    def logout(self, **fields):
        """LOGOUT (20). Поля: —. Разлогинить соединение. Payload — пустая map, как в референсном клиенте."""
        _spec = {}
        d = self.request(20, _build('Logout', _spec, fields))
        return _resp(d, [])

    def config(self, **fields):
        """CONFIG (22). Поля: pushToken:string?, pushTokens:[any]?, pushOptions:i64?, settings:{any}?, reset:bool?. Извлечено из декомпила 26.28. Verified mq2.java factory constructor with this.c==21 (super(x0c.r)); all 5 request keys confirmed. Response gl3.d=new gl3(3); dispatch gl3.h case 3 -> gl3.i(w2a) reads keys 'hash' and 'user'. Confirmed."""
        _spec = {'pushToken': 'string?', 'pushTokens': '[any]?', 'pushOptions': 'i64?', 'settings': '{any}?', 'reset': 'bool?'}
        d = self.request(22, _build('Config', _spec, fields))
        return _resp(d, ['hash', 'user'])

    def preset_avatars(self, **fields):
        """PRESET_AVATARS (25). Поля: —. Извлечено из декомпила 26.28. No request builder (empty). Response bya.i=new bya(7); dispatch bya.h case 7 -> bya.l(w2a) reads 'presetAvatars'. Confirmed."""
        _spec = {}
        d = self.request(25, _build('PresetAvatars', _spec, fields))
        return _resp(d, ['presetAvatars'])

    def contact_info(self, **fields):
        """CONTACT_INFO (32). Поля: contactIds:[any], chat_id:i64?. Извлечено из декомпила 26.28. Both request fields and the response field confirmed by literal setters/key comparisons. No changes."""
        _spec = {'contactIds': '[any]', 'chat_id': 'i64?'}
        d = self.request(32, _build('ContactInfo', _spec, fields))
        return _resp(d, ['contacts'])

    def contact_presence(self, **fields):
        """CONTACT_PRESENCE (35). Поля: —. Извлечено из декомпила 26.28. Request builder xw.java empty ctor xw(){super(x0c.e1);c=4;} sets no fields -> empty request. Response keys confirmed in bya g()."""
        _spec = {}
        d = self.request(35, _build('ContactPresence', _spec, fields))
        return _resp(d, ['presence', 'time'])

    def contact_info_by_phone(self, **fields):
        """CONTACT_INFO_BY_PHONE (46). Поля: —. Извлечено из декомпила 26.28. No request_builder in worklist. Response field confirmed. Unchanged."""
        _spec = {}
        d = self.request(46, _build('ContactInfoByPhone', _spec, fields))
        return _resp(d, ['contact'])

    def chat_info(self, **fields):
        """CHAT_INFO (48). Поля: chatIds:[any]. Извлечено из декомпила 26.28. Builder ctor confirmed (worklist had it null). Only chatIds set. No response parser."""
        _spec = {'chatIds': '[any]'}
        d = self.request(48, _build('ChatInfo', _spec, fields))
        return _resp(d, [])

    def chat_history(self, **fields):
        """CHAT_HISTORY (49). Поля: chatId:i64, postId:i64?, from:i64, forward:i32, forwardTime:i64, backward:i32, backwardTime:i64, getChat:bool, getMessages:bool, chatAccessToken:string?, itemType:string, interactive:bool. Извлечено из декомпила 26.28. mq2 ctor c=3 super(x0c.m1)=opcode 49 (x0c.java confirms (short)49). All 12 fields confirmed by literal setters. No response parser."""
        _spec = {'chatId': 'i64', 'postId': 'i64?', 'from': 'i64', 'forward': 'i32', 'forwardTime': 'i64', 'backward': 'i32', 'backwardTime': 'i64', 'getChat': 'bool', 'getMessages': 'bool', 'chatAccessToken': 'string?', 'itemType': 'string', 'interactive': 'bool'}
        d = self.request(49, _build('ChatHistory', _spec, fields))
        return _resp(d, [])

    def chat_mark(self, **fields):
        """CHAT_MARK (50). Поля: —. Извлечено из декомпила 26.28. No request_builder in worklist. Response field confirmed via smali. Unchanged."""
        _spec = {}
        d = self.request(50, _build('ChatMark', _spec, fields))
        return _resp(d, ['timestamp'])

    def chat_media(self, **fields):
        """CHAT_MEDIA (51). Поля: chatId:i64, messageId:i64, attachTypes:[any]?, forward:i32?, backward:i32?. Извлечено из декомпила 26.28. mq2 ctor c=9 super(x0c.o1)=opcode 51 (x0c.java confirms (short)51). All fields confirmed. No response parser."""
        _spec = {'chatId': 'i64', 'messageId': 'i64', 'attachTypes': '[any]?', 'forward': 'i32?', 'backward': 'i32?'}
        d = self.request(51, _build('ChatMedia', _spec, fields))
        return _resp(d, [])

    def chat_join(self, **fields):
        """CHAT_JOIN (57). Поля: —. Извлечено из декомпила 26.28. Verified: mx4 case 2 reads only 'chat'. No request builder."""
        _spec = {}
        d = self.request(57, _build('ChatJoin', _spec, fields))
        return _resp(d, ['chat'])

    def chat_members(self, **fields):
        """CHAT_MEMBERS (59). Поля: chatId:i64, type:string?, marker:i64?, count:i32?, query:string?. Извлечено из декомпила 26.28. Request verified against mq2 c=10 ctor; both response keys literally compared in bya case 2. Unchanged from draft."""
        _spec = {'chatId': 'i64', 'type': 'string?', 'marker': 'i64?', 'count': 'i32?', 'query': 'string?'}
        d = self.request(59, _build('ChatMembers', _spec, fields))
        return _resp(d, ['members', 'marker'])

    def chat_personal_config(self, **fields):
        """CHAT_PERSONAL_CONFIG (61). Поля: —. Извлечено из декомпила 26.28. cya.d=new cya(2). Dispatch cya.h(w2a) is 'Method not decompiled'; helpers m/n also not decompiled. Branch unreadable => empty."""
        _spec = {}
        d = self.request(61, _build('ChatPersonalConfig', _spec, fields))
        return _resp(d, ['chat'])

    def chat_livestream_info(self, **fields):
        """CHAT_LIVESTREAM_INFO (62). Поля: —. Извлечено из декомпила 26.28. Verified: gia case 2 reads only 'liveStreams'. No request builder."""
        _spec = {}
        d = self.request(62, _build('ChatLivestreamInfo', _spec, fields))
        return _resp(d, ['liveStreams'])

    def msg_send(self, **fields):
        """MSG_SEND (64). Поля: chatId:i64, message:OutgoingMessage, notify:bool?. Отправить сообщение в чат."""
        _spec = {'chatId': 'i64', 'message': 'OutgoingMessage', 'notify': 'bool?'}
        d = self.request(64, _build('MsgSend', _spec, fields))
        return _resp(d, ['chatId', 'message', 'unread', 'mark'])

    def msg_typing(self, **fields):
        """MSG_TYPING (65). Поля: chatId:i64, type:string. Индикатор набора текста."""
        _spec = {'chatId': 'i64', 'type': 'string'}
        d = self.request(65, _build('MsgTyping', _spec, fields))
        return _resp(d, [])

    def msg_get(self, **fields):
        """MSG_GET (71). Поля: chatId:i64, messageIds:[any]. Извлечено из декомпила 26.28. Request verified (d77 c=8). RESPONSE DROPPED vs draft: iya.h=new iya(5) but dispatch iya.h(w2a) is 'Method not decompiled', so the N=5 branch is unreadable. Draft's chatId/messages came from private helper d() (returns cla) which cannot be tied to N=5 by any decompiled code => empty per prefer-dropping-over-guessing."""
        _spec = {'chatId': 'i64', 'messageIds': '[any]'}
        d = self.request(71, _build('MsgGet', _spec, fields))
        return _resp(d, ['chatId', 'messages'])

    def video_chat_start(self, **fields):
        """VIDEO_CHAT_START (76). Поля: —. Извлечено из декомпила 26.28. ogc.d=new ogc(2); dispatch ogc.h(w2a) 'Method not decompiled', helper k not decompiled. Branch unreadable => empty. No request builder."""
        _spec = {}
        d = self.request(76, _build('VideoChatStart', _spec, fields))
        return _resp(d, ['type', 'callerId', 'callName', 'chatId', 'joinLink', 'conversationId'])

    def video_chat_start_active(self, **fields):
        """VIDEO_CHAT_START_ACTIVE (78). Поля: —. Извлечено из декомпила 26.28. gya.k=new gya(9); dispatch gya.h(w2a) 'Method not decompiled', helper n not decompiled. Readable helpers parse unrelated keys, cannot attribute N=9 => empty. No request builder."""
        _spec = {}
        d = self.request(78, _build('VideoChatStartActive', _spec, fields))
        return _resp(d, ['conversationId', 'internalCallerParams', 'rejectedParticipants'])

    def photo_upload(self, **fields):
        """PHOTO_UPLOAD (80). Поля: type:i32?, count:i32, profile:bool?. Извлечено из декомпила 26.28. Request verified against g9b c=1 ctor. Response parser ogc.b=new ogc(0); dispatch ogc.h(w2a) not decompiled => empty."""
        _spec = {'type': 'i32?', 'count': 'i32', 'profile': 'bool?'}
        d = self.request(80, _build('PhotoUpload', _spec, fields))
        return _resp(d, ['url'])

    def video_upload(self, **fields):
        """VIDEO_UPLOAD (82). Поля: type:i32, count:i32, uploaderType:i32. Извлечено из декомпила 26.28. Request verified (thg c=17). Response parser iya.l=new iya(9); dispatch iya.h(w2a) not decompiled => empty."""
        _spec = {'type': 'i32', 'count': 'i32', 'uploaderType': 'i32'}
        d = self.request(82, _build('VideoUpload', _spec, fields))
        return _resp(d, ['info', 'uploaderType'])

    def file_download(self, **fields):
        """FILE_DOWNLOAD (88). Поля: fileId:i64, chatId:i64, messageId:i64. Извлечено из декомпила 26.28. Request verified against mq2 c=28 ctor (fileId, chatId, messageId all i64). RESPONSE DROPPED vs draft: cya.f=new cya(4) but dispatch cya.h(w2a) is 'Method not decompiled', so N=4 branch unreadable. Draft's url/unsafe came from helper l() (returns ld6) which cannot be tied to N=4 by any decompiled code => empty per prefer-dropping-over-guessing."""
        _spec = {'fileId': 'i64', 'chatId': 'i64', 'messageId': 'i64'}
        d = self.request(88, _build('FileDownload', _spec, fields))
        return _resp(d, ['url', 'unsafe'])

    def get_comments_updates(self, **fields):
        """GET_COMMENTS_UPDATES (91). Поля: —. Извлечено из декомпила 26.28. No request builder -> empty map. Verified ogc.g=new ogc(5). CAVEAT: ogc.h(w2a) dispatch is dump-skipped so l()->N=5 mapping is unconfirmed; kept the field because l() is the only readable parser and its key matches this opcode's semantics."""
        _spec = {}
        d = self.request(91, _build('GetCommentsUpdates', _spec, fields))
        return _resp(d, ['commentsInfoUpdates'])

    def auth_login_restore_password(self, **fields):
        """AUTH_LOGIN_RESTORE_PASSWORD (101). Поля: —. Извлечено из декомпила 26.28. No request builder -> empty map. Verified bya.c=new bya(1); case 1 decompiled, only key is 'tokenAttrs'."""
        _spec = {}
        d = self.request(101, _build('AuthLoginRestorePassword', _spec, fields))
        return _resp(d, ['tokenAttrs'])

    def auth2fa_details(self, **fields):
        """AUTH_2FA_DETAILS (104). Поля: trackId:string?. Извлечено из декомпила 26.28. Request builder g9b(String) sets only trackId (conditionally). Response 'ogcVar' file null -> unreadable, empty response."""
        _spec = {'trackId': 'string?'}
        d = self.request(104, _build('Auth2faDetails', _spec, fields))
        return _resp(d, [])

    def external_callback(self, **fields):
        """EXTERNAL_CALLBACK (105). Поля: —. Извлечено из декомпила 26.28. No request builder -> empty map. Verified bya.f=new bya(4) -> i(w2a) decompiled; only keys 'botId','startParam'."""
        _spec = {}
        d = self.request(105, _build('ExternalCallback', _spec, fields))
        return _resp(d, ['botId', 'startParam'])

    def auth_verify_email(self, **fields):
        """AUTH_VERIFY_EMAIL (109). Поля: trackId:string, email:string?. Извлечено из декомпила 26.28. Request builder verified. Response cya.c=new cya(1); cya.h(w2a) is dump-skipped (unit count 1700) -> response unreadable."""
        _spec = {'trackId': 'string', 'email': 'string?'}
        d = self.request(109, _build('AuthVerifyEmail', _spec, fields))
        return _resp(d, [])

    def auth_check_email(self, **fields):
        """AUTH_CHECK_EMAIL (110). Поля: trackId:string, verifyCode:string. Извлечено из декомпила 26.28. Request builder ub0 (not a factory), both fields always set. Response tyf.d=new tyf(1), case 1 decompiled; only keys 'trackId','email'."""
        _spec = {'trackId': 'string', 'verifyCode': 'string'}
        d = self.request(110, _build('AuthCheckEmail', _spec, fields))
        return _resp(d, ['trackId', 'email'])

    def auth_set2fa(self, **fields):
        """AUTH_SET_2FA (111). Поля: trackId:string, password:string?, hint:string?, remove2fa:bool?, expectedCapabilities:[any]. Извлечено из декомпила 26.28. Request builder verified; corrected expectedCapabilities from [any] to [i8] (list is built from Byte values). Response cd0.a=new cd0() (no parse method) -> unreadable."""
        _spec = {'trackId': 'string', 'password': 'string?', 'hint': 'string?', 'remove2fa': 'bool?', 'expectedCapabilities': '[any]'}
        d = self.request(111, _build('AuthSet2fa', _spec, fields))
        return _resp(d, [])

    def auth_create_track(self, **fields):
        """AUTH_CREATE_TRACK (112). Поля: type:i32. Извлечено из декомпила 26.28. Request builder no-arg sets only 'type' (Java short). Response mx4.c=new mx4(1), case 1 decompiled; only key 'trackId'."""
        _spec = {'type': 'i32'}
        d = self.request(112, _build('AuthCreateTrack', _spec, fields))
        return _resp(d, ['trackId'])

    def auth_check_password(self, **fields):
        """AUTH_CHECK_PASSWORD (113). Поля: —. Извлечено из декомпила 26.28. No request builder -> empty map. Response gl3.b=new gl3(1), case 1 decompiled; only key 'trackId'."""
        _spec = {}
        d = self.request(113, _build('AuthCheckPassword', _spec, fields))
        return _resp(d, ['trackId'])

    def auth_login_check_password(self, **fields):
        """AUTH_LOGIN_CHECK_PASSWORD (115). Поля: trackId:string, password:string. Облачный пароль (2FA) после `passwordChallenge`. В ответе — тот же `tokenAttrs.LOGIN`."""
        _spec = {'trackId': 'string', 'password': 'string'}
        d = self.request(115, _build('AuthLoginCheckPassword', _spec, fields))
        return _resp(d, ['tokenAttrs', 'profile', 'error'])

    def notif_msg_delete(self, **fields):
        """NOTIF_MSG_DELETE (142). Поля: —. Извлечено из декомпила 26.28. Verified via iya.h smali (iya.i=new iya(6); packed-switch 0x1 -> a=6 = pswitch_2). Builds t0b(nm2,[J,Z). All three keys confirmed; nothing dropped."""
        _spec = {}
        d = self.request(142, _build('NotifMsgDelete', _spec, fields))
        return _resp(d, ['chat', 'ttl', 'messageIds'])

    def bot_info(self, **fields):
        """BOT_INFO (145). Поля: —. Извлечено из декомпила 26.28. Verified via gya.h smali (gya.c=new gya(1); packed-switch 0x1 -> a=1 = pswitch_7, inlined). All three keys confirmed."""
        _spec = {}
        d = self.request(145, _build('BotInfo', _spec, fields))
        return _resp(d, ['contact', 'startMessage', 'commands'])

    def notif_msg_delayed(self, **fields):
        """NOTIF_MSG_DELAYED (154). Поля: —. Извлечено из декомпила 26.28. Verified via gya.n smali (a=6 = pswitch_3 -> n(w2a)). Builds p0b. All six keys confirmed with correct types."""
        _spec = {}
        d = self.request(154, _build('NotifMsgDelayed', _spec, fields))
        return _resp(d, ['message', 'userId', 'updateTypeId', 'lastDelayedUpdateTime', 'chatId', 'messageIds'])

    def ok_token(self, **fields):
        """OK_TOKEN (158). Поля: value:string, userId:i64. Извлечено из декомпила 26.28. Request g9b(String,long) c=0 confirmed. Response zg6.h smali: packed-switch only maps a=1; a=7 falls to default branch reading token/token_refresh_ts/token_lifetime_ts into h9b. 4th long is client nanoTime, not a wire field."""
        _spec = {'value': 'string', 'userId': 'i64'}
        d = self.request(158, _build('OkToken', _spec, fields))
        return _resp(d, ['token', 'token_refresh_ts', 'token_lifetime_ts'])

    def notif_profile(self, **fields):
        """NOTIF_PROFILE (159). Поля: —. Извлечено из декомпила 26.28. Verified via dfk.j jadx (dfk.g=new dfk(6); switch case 6 -> j). Single key confirmed."""
        _spec = {}
        d = self.request(159, _build('NotifProfile', _spec, fields))
        return _resp(d, ['profile'])

    def web_app_init_data(self, **fields):
        """WEB_APP_INIT_DATA (160). Поля: —. Извлечено из декомпила 26.28. Verified via zl0.h smali (zl0.l=new zl0(10); packed-switch 0x0 covers 0..7, a=10 falls to default = pswitch_0) building n2j(url,query_id)."""
        _spec = {}
        d = self.request(160, _build('WebAppInitData', _spec, fields))
        return _resp(d, ['url', 'query_id'])

    def complain(self, **fields):
        """COMPLAIN (161). Поля: typeId:i32, reasonId:i32, ids:[any], parentId:i64?, postId:i64?, details:string?. Извлечено из декомпила 26.28. Request mq2(cw3,byte,long[],Long,String,Long) c=20 confirmed. Response iya.h smali a=2 = pswitch_5 reads 'success'. All fields confirmed."""
        _spec = {'typeId': 'i32', 'reasonId': 'i32', 'ids': '[any]', 'parentId': 'i64?', 'postId': 'i64?', 'details': 'string?'}
        d = self.request(161, _build('Complain', _spec, fields))
        return _resp(d, ['success'])

    def complain_reasons_get(self, **fields):
        """COMPLAIN_REASONS_GET (162). Поля: —. Извлечено из декомпила 26.28. Verified via dfk.h jadx case 2 (dfk.c=new dfk(2)). Both keys confirmed; builds bw3(long,List)."""
        _spec = {}
        d = self.request(162, _build('ComplainReasonsGet', _spec, fields))
        return _resp(d, ['complainSync', 'complains'])

    def call_history(self, **fields):
        """CALL_HISTORY (163). Поля: callHistorySync:i64. Извлечено из декомпила 26.28. Request g9b(long) c=24 confirmed. Response zl0.h smali a=2 = pswitch_4 building lf1(List,long,boolean). All confirmed."""
        _spec = {'callHistorySync': 'i64'}
        d = self.request(163, _build('CallHistory', _spec, fields))
        return _resp(d, ['reset', 'callHistoryItems', 'callHistorySync'])

    def video_chat_delete_history(self, **fields):
        """VIDEO_CHAT_DELETE_HISTORY (164). Поля: historyIds:[any]?. Извлечено из декомпила 26.28. В клиенте опкод назван CALL_HISTORY_CLEAR. Request g9b(long[]) c=23 confirmed. Response dfk.h jadx case 1 returns kf1.c singleton (empty, no parsed fields)."""
        _spec = {'historyIds': '[any]?'}
        d = self.request(164, _build('VideoChatDeleteHistory', _spec, fields))
        return _resp(d, [])

    def notif_call_history(self, **fields):
        """NOTIF_CALL_HISTORY (165). Поля: —. Извлечено из декомпила 26.28. Verified via bya.k smali (bya.h=new bya(6); switch case 6 -> k). Builds sza(JJ,rza,List,[J). All five keys confirmed."""
        _spec = {}
        d = self.request(165, _build('NotifCallHistory', _spec, fields))
        return _resp(d, ['historyIds', 'callHistoryItems', 'prevCallHistorySync', 'callHistorySync', 'action'])

    def video_chat_join_by_link(self, **fields):
        """VIDEO_CHAT_JOIN_BY_LINK (166). Поля: —. Извлечено из декомпила 26.28. В клиенте опкод назван VIDEO_CHAT_JOIN. Verified via fya.h jadx (fya.k=new fya(9); a=9 not an explicit case -> default branch shared with cases 3/5) building pci(conversationId,internalParams)."""
        _spec = {}
        d = self.request(166, _build('VideoChatJoinByLink', _spec, fields))
        return _resp(d, ['conversationId', 'internalParams'])

    def msg_get_reactions(self, **fields):
        """MSG_GET_REACTIONS (180). Поля: —. Извлечено из декомпила 26.28. Verified via zl0.n smali (zl0.h=new zl0(6); a=6 = pswitch_2 -> n(w2a)). Single map key confirmed."""
        _spec = {}
        d = self.request(180, _build('MsgGetReactions', _spec, fields))
        return _resp(d, ['messagesReactions'])

    def profile_delete(self, **fields):
        """PROFILE_DELETE (199). Поля: delete:bool, type:i32?. Извлечено из декомпила 26.28. Request d77(boolean,int) c=23 confirmed. Response fya.l jadx (fya.i=new fya(7); case 7 -> l). Confirmed."""
        _spec = {'delete': 'bool', 'type': 'i32?'}
        d = self.request(199, _build('ProfileDelete', _spec, fields))
        return _resp(d, ['timestamp'])

    def profile_delete_time(self, **fields):
        """PROFILE_DELETE_TIME (200). Поля: —. Извлечено из декомпила 26.28. Request d77() c=24 sets no fields => empty request. Response gya.o smali (gya.i=new gya(7); a=7 = pswitch_2 -> o(w2a)). Single key confirmed."""
        _spec = {}
        d = self.request(200, _build('ProfileDeleteTime', _spec, fields))
        return _resp(d, ['timestamp'])

    def audio_transcription(self, **fields):
        """AUDIO_TRANSCRIPTION (202). Поля: mediaId:i64, messageId:i64, chatId:i64. Извлечено из декомпила 26.28. В клиенте опкод назван TRANSCRIBE_MEDIA. Request thg(long,long,long) super(x0c.G3) c=12 confirmed (chatId included). Response cmh.a singleton builds bmh(String,bnh). Both keys confirmed."""
        _spec = {'mediaId': 'i64', 'messageId': 'i64', 'chatId': 'i64'}
        d = self.request(202, _build('AudioTranscription', _spec, fields))
        return _resp(d, ['transcription', 'transcriptionStatus'])

    def photo_url_refresh(self, **fields):
        """PHOTO_URL_REFRESH (203). Поля: —. Извлечено из декомпила 26.28. Verified via gl3.j jadx (gl3.h=new gl3(7); case 7 -> j). Single key confirmed."""
        _spec = {}
        d = self.request(203, _build('PhotoUrlRefresh', _spec, fields))
        return _resp(d, ['media'])

    def stories_list(self, **fields):
        """STORIES_LIST (208). Поля: —. Извлечено из декомпила 26.28. Verified via fya.m jadx (fya.j=new fya(8); case 8 -> m) building lig(zpa,String). Both keys confirmed."""
        _spec = {}
        d = self.request(208, _build('StoriesList', _spec, fields))
        return _resp(d, ['storiesPreviews', 'cursor'])

    def stories_get_by_owner(self, **fields):
        """STORIES_GET_BY_OWNER (210). Поля: —. Извлечено из декомпила 26.28. В клиенте опкод назван STORIES_GET_BY_OWNER_ID. Verified both keys and the return new whg(peerStories, storiesPreviews) in the mx4 N=8 default branch. Kept as-is."""
        _spec = {}
        d = self.request(210, _build('StoriesGetByOwner', _spec, fields))
        return _resp(d, ['peerStories', 'storiesPreviews'])

    def stories_get_stats(self, **fields):
        """STORIES_GET_STATS (211). Поля: —. Извлечено из декомпила 26.28. Verified key 'storyStats' and return new zhg(storyStats) in bya N=8 default branch. Kept."""
        _spec = {}
        d = self.request(211, _build('StoriesGetStats', _spec, fields))
        return _resp(d, ['storyStats'])

    def stories_react(self, **fields):
        """STORIES_REACT (213). Поля: —. Извлечено из декомпила 26.28. Verified: parser iya.k => new iya(8); iya.h(w2a) is 'Method not decompiled'. Branch unreadable. Response empty (confirmed)."""
        _spec = {}
        d = self.request(213, _build('StoriesReact', _spec, fields))
        return _resp(d, ['success'])

    def stories_mark(self, **fields):
        """STORIES_MARK (214). Поля: —. Извлечено из декомпила 26.28. Verified: parser gya.j => new gya(8); gya.h(w2a) is 'Method not decompiled'. Branch unreadable. Response empty (confirmed)."""
        _spec = {}
        d = self.request(214, _build('StoriesMark', _spec, fields))
        return _resp(d, ['success'])

    def stories_send(self, **fields):
        """STORIES_SEND (215). Поля: —. Извлечено из декомпила 26.28. Verified both keys and return new ojg(stories, storiesPreview) in dfk N=8 default branch. Kept."""
        _spec = {}
        d = self.request(215, _build('StoriesSend', _spec, fields))
        return _resp(d, ['stories', 'storiesPreview'])

    def notif_stories_update(self, **fields):
        """NOTIF_STORIES_UPDATE (216). Поля: —. Извлечено из декомпила 26.28. Verified: parser zl0.i => new zl0(7); zl0.h(w2a) is 'Method not decompiled'. Branch unreadable. Response empty (confirmed)."""
        _spec = {}
        d = self.request(216, _build('NotifStoriesUpdate', _spec, fields))
        return _resp(d, ['storiesPreview'])

    def stories_edit(self, **fields):
        """STORIES_EDIT (217). Поля: —. Извлечено из декомпила 26.28. Verified key 'story' and return new uhg(story) in gl3 N=8 default branch. Kept."""
        _spec = {}
        d = self.request(217, _build('StoriesEdit', _spec, fields))
        return _resp(d, ['story'])

    def stories_delete(self, **fields):
        """STORIES_DELETE (218). Поля: —. Извлечено из декомпила 26.28. Verified key 'storiesPreview' and return new phg(storiesPreview) in tyf N=8 default branch. Kept."""
        _spec = {}
        d = self.request(218, _build('StoriesDelete', _spec, fields))
        return _resp(d, ['storiesPreview'])

    def stories_get_by_story_id(self, **fields):
        """STORIES_GET_BY_STORY_ID (220). Поля: —. Извлечено из декомпила 26.28. Verified key 'stories' and return new xhg(stories) in gia N=8 default branch. Kept."""
        _spec = {}
        d = self.request(220, _build('StoriesGetByStoryId', _spec, fields))
        return _resp(d, ['stories'])

    def org_info(self, **fields):
        """ORG_INFO (256). Поля: organizationIds:[any]?. Извлечено из декомпила 26.28. Verified request key in z2c (rpa confirmed to wrap long[] => [i64]) and response key 'organizations' in tyf case-7 branch. Kept."""
        _spec = {'organizationIds': '[any]?'}
        d = self.request(256, _build('OrgInfo', _spec, fields))
        return _resp(d, ['organizations'])

    def chat_reactions_settings_set(self, **fields):
        """CHAT_REACTIONS_SETTINGS_SET (257). Поля: —. Извлечено из декомпила 26.28. Verified key 'chatReactionsSettings' and return new t53 in fya case-2 branch. Kept."""
        _spec = {}
        d = self.request(257, _build('ChatReactionsSettingsSet', _spec, fields))
        return _resp(d, ['chatReactionsSettings'])

    def reactions_settings_get_by_chat_id(self, **fields):
        """REACTIONS_SETTINGS_GET_BY_CHAT_ID (258). Поля: —. Извлечено из декомпила 26.28. Verified key 'chatReactionsSettings' and return new or2 in gl3 case-2 branch. Kept."""
        _spec = {}
        d = self.request(258, _build('ReactionsSettingsGetByChatId', _spec, fields))
        return _resp(d, ['chatReactionsSettings'])

    def folders_get(self, **fields):
        """FOLDERS_GET (272). Поля: folderSync:i64. Извлечено из декомпила 26.28. Verified request: zr6 sets folderSync via f(long,String) => i64. Response parser iya.g => new iya(4); iya.h(w2a) is 'Method not decompiled', N=4 branch unreadable; response empty."""
        _spec = {'folderSync': 'i64'}
        d = self.request(272, _build('FoldersGet', _spec, fields))
        return _resp(d, ['folderSync', 'folders', 'foldersOrder', 'allFilterExcludeFolders'])

    def folders_get_by_id(self, **fields):
        """FOLDERS_GET_BY_ID (273). Поля: —. Извлечено из декомпила 26.28. No request builder -> empty request. DROPPED 'folders': gya.h(w2a) dispatch is 'Method not decompiled' (UnsupportedOperationException), so the N=4 branch cannot be confirmed to call helper m() which reads 'folders'. Attribution was inferred, not proven -> response emptied."""
        _spec = {}
        d = self.request(273, _build('FoldersGetById', _spec, fields))
        return _resp(d, ['folders'])

    def folders_update(self, **fields):
        """FOLDERS_UPDATE (274). Поля: id:string, title:string, include:[any]?, favorites:[any]?, filters:[any]?, options:[any]?. Извлечено из декомпила 26.28. Builder kt6 verified (all 6 keys confirmed, types match). Response parser zl0.g (N=5) dispatch not decompiled; no fields confirmable -> empty."""
        _spec = {'id': 'string', 'title': 'string', 'include': '[any]?', 'favorites': '[any]?', 'filters': '[any]?', 'options': '[any]?'}
        d = self.request(274, _build('FoldersUpdate', _spec, fields))
        return _resp(d, ['folder', 'folderSync', 'foldersOrder'])

    def folders_reorder(self, **fields):
        """FOLDERS_REORDER (275). Поля: foldersOrder:[any]. Извлечено из декомпила 26.28. Builder dt6 and response dfk case4 both confirmed on-path."""
        _spec = {'foldersOrder': '[any]'}
        d = self.request(275, _build('FoldersReorder', _spec, fields))
        return _resp(d, ['folderSync'])

    def folders_delete(self, **fields):
        """FOLDERS_DELETE (276). Поля: folderIds:[any]. Извлечено из декомпила 26.28. Builder wr6 and response fya case4 both confirmed on-path."""
        _spec = {'folderIds': '[any]'}
        d = self.request(276, _build('FoldersDelete', _spec, fields))
        return _resp(d, ['folderSync'])

    def notif_folders(self, **fields):
        """NOTIF_FOLDERS (277). Поля: —. Извлечено из декомпила 26.28. No request builder (notification). Response parser cya.h (N=6) dispatch not decompiled; no fields confirmable."""
        _spec = {}
        d = self.request(277, _build('NotifFolders', _spec, fields))
        return _resp(d, ['folderSync', 'folders', 'foldersOrder', 'allFilterExcludeFolders'])

    def transcription_result(self, **fields):
        """TRANSCRIPTION_RESULT (293). Поля: —. Извлечено из декомпила 26.28. В клиенте опкод назван NOTIF_TRANSCRIPTION. No request builder (notification). Response parser g1b/vl3.h not decompiled. Unrecoverable."""
        _spec = {}
        d = self.request(293, _build('TranscriptionResult', _spec, fields))
        return _resp(d, ['chatId', 'messageId', 'mediaId', 'transcription', 'transcriptionStatus'])

    def chat_suggest(self, **fields):
        """CHAT_SUGGEST (300). Поля: folderId:string?, userChatIds:[any]?. Извлечено из декомпила 26.28. Builder xw(String,long[]) c=2 confirmed. Response gya.d (N=2) dispatch not decompiled -> empty."""
        _spec = {'folderId': 'string?', 'userChatIds': '[any]?'}
        d = self.request(300, _build('ChatSuggest', _spec, fields))
        return _resp(d, ['chats'])

    def banners_get(self, **fields):
        """BANNERS_GET (302). Поля: bannersSync:i64. Извлечено из декомпила 26.28. Builder xl0 confirmed. Response zl0.b (N=0) dispatch not decompiled -> empty."""
        _spec = {'bannersSync': 'i64'}
        d = self.request(302, _build('BannersGet', _spec, fields))
        return _resp(d, [])

    def send_vote(self, **fields):
        """SEND_VOTE (304). Поля: chatId:i64, pollId:i64, messageId:i64, answersIds:[any]. Извлечено из декомпила 26.28. Builder luc and response gia case7->m() both confirmed on-path."""
        _spec = {'chatId': 'i64', 'pollId': 'i64', 'messageId': 'i64', 'answersIds': '[any]'}
        d = self.request(304, _build('SendVote', _spec, fields))
        return _resp(d, ['state'])

    def voters_list_by_answer(self, **fields):
        """VOTERS_LIST_BY_ANSWER (305). Поля: chatId:i64, pollId:i64, messageId:i64, answerId:i32, marker:i64?. Извлечено из декомпила 26.28. Builder suc confirmed (all 5 keys). Response qma.g (N=7) parse methods not decompiled -> empty."""
        _spec = {'chatId': 'i64', 'pollId': 'i64', 'messageId': 'i64', 'answerId': 'i32', 'marker': 'i64?'}
        d = self.request(305, _build('VotersListByAnswer', _spec, fields))
        return _resp(d, [])

    def get_poll_updates(self, **fields):
        """GET_POLL_UPDATES (306). Поля: chatId:i64, polls:[any]. Извлечено из декомпила 26.28. Builder gla and response dfk case5->i() both confirmed on-path."""
        _spec = {'chatId': 'i64', 'polls': '[any]'}
        d = self.request(306, _build('GetPollUpdates', _spec, fields))
        return _resp(d, ['polls'])


# Опкод по wire-имени, если нужен номер напрямую.
OPCODES = {'AUDIO_TRANSCRIPTION': 202, 'AUTH': 18, 'AUTH_2FA_DETAILS': 104, 'AUTH_CHECK_EMAIL': 110, 'AUTH_CHECK_PASSWORD': 113, 'AUTH_CREATE_TRACK': 112, 'AUTH_LOGIN_CHECK_PASSWORD': 115, 'AUTH_LOGIN_RESTORE_PASSWORD': 101, 'AUTH_REQUEST': 17, 'AUTH_SET_2FA': 111, 'AUTH_VERIFY_EMAIL': 109, 'BANNERS_GET': 302, 'BOT_INFO': 145, 'CALL_HISTORY': 163, 'CHAT_HISTORY': 49, 'CHAT_INFO': 48, 'CHAT_JOIN': 57, 'CHAT_LIVESTREAM_INFO': 62, 'CHAT_MARK': 50, 'CHAT_MEDIA': 51, 'CHAT_MEMBERS': 59, 'CHAT_PERSONAL_CONFIG': 61, 'CHAT_REACTIONS_SETTINGS_SET': 257, 'CHAT_SUGGEST': 300, 'COMPLAIN': 161, 'COMPLAIN_REASONS_GET': 162, 'CONFIG': 22, 'CONTACT_INFO': 32, 'CONTACT_INFO_BY_PHONE': 46, 'CONTACT_PRESENCE': 35, 'EXTERNAL_CALLBACK': 105, 'FILE_DOWNLOAD': 88, 'FOLDERS_DELETE': 276, 'FOLDERS_GET': 272, 'FOLDERS_GET_BY_ID': 273, 'FOLDERS_REORDER': 275, 'FOLDERS_UPDATE': 274, 'GET_COMMENTS_UPDATES': 91, 'GET_POLL_UPDATES': 306, 'LOGIN': 19, 'LOGOUT': 20, 'MSG_GET': 71, 'MSG_GET_REACTIONS': 180, 'MSG_SEND': 64, 'MSG_TYPING': 65, 'NOTIF_CALL_HISTORY': 165, 'NOTIF_FOLDERS': 277, 'NOTIF_MSG_DELAYED': 154, 'NOTIF_MSG_DELETE': 142, 'NOTIF_PROFILE': 159, 'NOTIF_STORIES_UPDATE': 216, 'OK_TOKEN': 158, 'ORG_INFO': 256, 'PHOTO_UPLOAD': 80, 'PHOTO_URL_REFRESH': 203, 'PRESET_AVATARS': 25, 'PROFILE_DELETE': 199, 'PROFILE_DELETE_TIME': 200, 'REACTIONS_SETTINGS_GET_BY_CHAT_ID': 258, 'SEND_VOTE': 304, 'STORIES_DELETE': 218, 'STORIES_EDIT': 217, 'STORIES_GET_BY_OWNER': 210, 'STORIES_GET_BY_STORY_ID': 220, 'STORIES_GET_STATS': 211, 'STORIES_LIST': 208, 'STORIES_MARK': 214, 'STORIES_REACT': 213, 'STORIES_SEND': 215, 'TRANSCRIPTION_RESULT': 293, 'VIDEO_CHAT_DELETE_HISTORY': 164, 'VIDEO_CHAT_JOIN_BY_LINK': 166, 'VIDEO_CHAT_START': 76, 'VIDEO_CHAT_START_ACTIVE': 78, 'VIDEO_UPLOAD': 82, 'VOTERS_LIST_BY_ANSWER': 305, 'WEB_APP_INIT_DATA': 160}
