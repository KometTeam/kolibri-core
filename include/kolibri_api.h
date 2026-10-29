/*
 * @generated tools/gen_schema.py -- не редактировать руками.
 *
 * Источник: файлы schema, расширение toml
 * Перегенерация: python3 tools/gen_schema.py
 */


#ifndef KOLIBRI_API_H
#define KOLIBRI_API_H

#include "kolibri_ops.h"
#include "kolibri_session.h"
#include "kolibri_client.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * По методу на операцию. kb_session_<op> кодирует запрос из
 * типизированной структуры и шлёт его (I/O ведёшь сам, event-loop);
 * kb_call_<op> вдобавок сам качает I/O до ответа и разбирает его.
 * Пейлоады руками не пишешь. Ответ kb_call_<op> живёт до следующего
 * вызова (kb_str/kb_raw смотрят внутрь буфера клиента).
 */

kb_status kb_send_auth_request(kb_session *s, const kb_auth_request *v, uint16_t *seq);
kb_status kb_call_auth_request(kb_client *c, const kb_auth_request *v, kb_auth_request_response *resp);

kb_status kb_send_auth_verify(kb_session *s, const kb_auth_verify *v, uint16_t *seq);
kb_status kb_call_auth_verify(kb_client *c, const kb_auth_verify *v, kb_auth_verify_response *resp);

kb_status kb_send_login(kb_session *s, const kb_login *v, uint16_t *seq);
kb_status kb_call_login(kb_client *c, const kb_login *v, kb_login_response *resp);

kb_status kb_send_logout(kb_session *s, uint16_t *seq);
kb_status kb_call_logout(kb_client *c, kb_logout_response *resp);

kb_status kb_send_config(kb_session *s, const kb_config *v, uint16_t *seq);
kb_status kb_call_config(kb_client *c, const kb_config *v, kb_config_response *resp);

kb_status kb_send_preset_avatars(kb_session *s, uint16_t *seq);
kb_status kb_call_preset_avatars(kb_client *c, kb_preset_avatars_response *resp);

kb_status kb_send_contact_info(kb_session *s, const kb_contact_info *v, uint16_t *seq);
kb_status kb_call_contact_info(kb_client *c, const kb_contact_info *v, kb_contact_info_response *resp);

kb_status kb_send_contact_presence(kb_session *s, uint16_t *seq);
kb_status kb_call_contact_presence(kb_client *c, kb_contact_presence_response *resp);

kb_status kb_send_contact_info_by_phone(kb_session *s, uint16_t *seq);
kb_status kb_call_contact_info_by_phone(kb_client *c, kb_contact_info_by_phone_response *resp);

kb_status kb_send_chat_info(kb_session *s, const kb_chat_info *v, uint16_t *seq);
kb_status kb_call_chat_info(kb_client *c, const kb_chat_info *v, kb_chat_info_response *resp);

kb_status kb_send_chat_history(kb_session *s, const kb_chat_history *v, uint16_t *seq);
kb_status kb_call_chat_history(kb_client *c, const kb_chat_history *v, kb_chat_history_response *resp);

kb_status kb_send_chat_mark(kb_session *s, uint16_t *seq);
kb_status kb_call_chat_mark(kb_client *c, kb_chat_mark_response *resp);

kb_status kb_send_chat_media(kb_session *s, const kb_chat_media *v, uint16_t *seq);
kb_status kb_call_chat_media(kb_client *c, const kb_chat_media *v, kb_chat_media_response *resp);

kb_status kb_send_chat_join(kb_session *s, uint16_t *seq);
kb_status kb_call_chat_join(kb_client *c, kb_chat_join_response *resp);

kb_status kb_send_chat_members(kb_session *s, const kb_chat_members *v, uint16_t *seq);
kb_status kb_call_chat_members(kb_client *c, const kb_chat_members *v, kb_chat_members_response *resp);

kb_status kb_send_chat_personal_config(kb_session *s, uint16_t *seq);
kb_status kb_call_chat_personal_config(kb_client *c, kb_chat_personal_config_response *resp);

kb_status kb_send_chat_livestream_info(kb_session *s, uint16_t *seq);
kb_status kb_call_chat_livestream_info(kb_client *c, kb_chat_livestream_info_response *resp);

kb_status kb_send_msg_send(kb_session *s, const kb_msg_send *v, uint16_t *seq);
kb_status kb_call_msg_send(kb_client *c, const kb_msg_send *v, kb_msg_send_response *resp);

kb_status kb_send_msg_typing(kb_session *s, const kb_msg_typing *v, uint16_t *seq);
kb_status kb_call_msg_typing(kb_client *c, const kb_msg_typing *v, kb_msg_typing_response *resp);

kb_status kb_send_msg_get(kb_session *s, const kb_msg_get *v, uint16_t *seq);
kb_status kb_call_msg_get(kb_client *c, const kb_msg_get *v, kb_msg_get_response *resp);

kb_status kb_send_video_chat_start(kb_session *s, uint16_t *seq);
kb_status kb_call_video_chat_start(kb_client *c, kb_video_chat_start_response *resp);

kb_status kb_send_video_chat_start_active(kb_session *s, uint16_t *seq);
kb_status kb_call_video_chat_start_active(kb_client *c, kb_video_chat_start_active_response *resp);

kb_status kb_send_photo_upload(kb_session *s, const kb_photo_upload *v, uint16_t *seq);
kb_status kb_call_photo_upload(kb_client *c, const kb_photo_upload *v, kb_photo_upload_response *resp);

kb_status kb_send_video_upload(kb_session *s, const kb_video_upload *v, uint16_t *seq);
kb_status kb_call_video_upload(kb_client *c, const kb_video_upload *v, kb_video_upload_response *resp);

kb_status kb_send_file_download(kb_session *s, const kb_file_download *v, uint16_t *seq);
kb_status kb_call_file_download(kb_client *c, const kb_file_download *v, kb_file_download_response *resp);

kb_status kb_send_get_comments_updates(kb_session *s, uint16_t *seq);
kb_status kb_call_get_comments_updates(kb_client *c, kb_get_comments_updates_response *resp);

kb_status kb_send_auth_login_restore_password(kb_session *s, uint16_t *seq);
kb_status kb_call_auth_login_restore_password(kb_client *c, kb_auth_login_restore_password_response *resp);

kb_status kb_send_auth2fa_details(kb_session *s, const kb_auth2fa_details *v, uint16_t *seq);
kb_status kb_call_auth2fa_details(kb_client *c, const kb_auth2fa_details *v, kb_auth2fa_details_response *resp);

kb_status kb_send_external_callback(kb_session *s, uint16_t *seq);
kb_status kb_call_external_callback(kb_client *c, kb_external_callback_response *resp);

kb_status kb_send_auth_verify_email(kb_session *s, const kb_auth_verify_email *v, uint16_t *seq);
kb_status kb_call_auth_verify_email(kb_client *c, const kb_auth_verify_email *v, kb_auth_verify_email_response *resp);

kb_status kb_send_auth_check_email(kb_session *s, const kb_auth_check_email *v, uint16_t *seq);
kb_status kb_call_auth_check_email(kb_client *c, const kb_auth_check_email *v, kb_auth_check_email_response *resp);

kb_status kb_send_auth_set2fa(kb_session *s, const kb_auth_set2fa *v, uint16_t *seq);
kb_status kb_call_auth_set2fa(kb_client *c, const kb_auth_set2fa *v, kb_auth_set2fa_response *resp);

kb_status kb_send_auth_create_track(kb_session *s, const kb_auth_create_track *v, uint16_t *seq);
kb_status kb_call_auth_create_track(kb_client *c, const kb_auth_create_track *v, kb_auth_create_track_response *resp);

kb_status kb_send_auth_check_password(kb_session *s, uint16_t *seq);
kb_status kb_call_auth_check_password(kb_client *c, kb_auth_check_password_response *resp);

kb_status kb_send_auth_login_check_password(kb_session *s, const kb_auth_login_check_password *v, uint16_t *seq);
kb_status kb_call_auth_login_check_password(kb_client *c, const kb_auth_login_check_password *v, kb_auth_login_check_password_response *resp);

kb_status kb_send_notif_msg_delete(kb_session *s, uint16_t *seq);
kb_status kb_call_notif_msg_delete(kb_client *c, kb_notif_msg_delete_response *resp);

kb_status kb_send_bot_info(kb_session *s, uint16_t *seq);
kb_status kb_call_bot_info(kb_client *c, kb_bot_info_response *resp);

kb_status kb_send_notif_msg_delayed(kb_session *s, uint16_t *seq);
kb_status kb_call_notif_msg_delayed(kb_client *c, kb_notif_msg_delayed_response *resp);

kb_status kb_send_ok_token(kb_session *s, const kb_ok_token *v, uint16_t *seq);
kb_status kb_call_ok_token(kb_client *c, const kb_ok_token *v, kb_ok_token_response *resp);

kb_status kb_send_notif_profile(kb_session *s, uint16_t *seq);
kb_status kb_call_notif_profile(kb_client *c, kb_notif_profile_response *resp);

kb_status kb_send_web_app_init_data(kb_session *s, uint16_t *seq);
kb_status kb_call_web_app_init_data(kb_client *c, kb_web_app_init_data_response *resp);

kb_status kb_send_complain(kb_session *s, const kb_complain *v, uint16_t *seq);
kb_status kb_call_complain(kb_client *c, const kb_complain *v, kb_complain_response *resp);

kb_status kb_send_complain_reasons_get(kb_session *s, uint16_t *seq);
kb_status kb_call_complain_reasons_get(kb_client *c, kb_complain_reasons_get_response *resp);

kb_status kb_send_call_history(kb_session *s, const kb_call_history *v, uint16_t *seq);
kb_status kb_call_call_history(kb_client *c, const kb_call_history *v, kb_call_history_response *resp);

kb_status kb_send_video_chat_delete_history(kb_session *s, const kb_video_chat_delete_history *v, uint16_t *seq);
kb_status kb_call_video_chat_delete_history(kb_client *c, const kb_video_chat_delete_history *v, kb_video_chat_delete_history_response *resp);

kb_status kb_send_notif_call_history(kb_session *s, uint16_t *seq);
kb_status kb_call_notif_call_history(kb_client *c, kb_notif_call_history_response *resp);

kb_status kb_send_video_chat_join_by_link(kb_session *s, uint16_t *seq);
kb_status kb_call_video_chat_join_by_link(kb_client *c, kb_video_chat_join_by_link_response *resp);

kb_status kb_send_msg_get_reactions(kb_session *s, uint16_t *seq);
kb_status kb_call_msg_get_reactions(kb_client *c, kb_msg_get_reactions_response *resp);

kb_status kb_send_profile_delete(kb_session *s, const kb_profile_delete *v, uint16_t *seq);
kb_status kb_call_profile_delete(kb_client *c, const kb_profile_delete *v, kb_profile_delete_response *resp);

kb_status kb_send_profile_delete_time(kb_session *s, uint16_t *seq);
kb_status kb_call_profile_delete_time(kb_client *c, kb_profile_delete_time_response *resp);

kb_status kb_send_audio_transcription(kb_session *s, const kb_audio_transcription *v, uint16_t *seq);
kb_status kb_call_audio_transcription(kb_client *c, const kb_audio_transcription *v, kb_audio_transcription_response *resp);

kb_status kb_send_photo_url_refresh(kb_session *s, uint16_t *seq);
kb_status kb_call_photo_url_refresh(kb_client *c, kb_photo_url_refresh_response *resp);

kb_status kb_send_stories_list(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_list(kb_client *c, kb_stories_list_response *resp);

kb_status kb_send_stories_get_by_owner(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_get_by_owner(kb_client *c, kb_stories_get_by_owner_response *resp);

kb_status kb_send_stories_get_stats(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_get_stats(kb_client *c, kb_stories_get_stats_response *resp);

kb_status kb_send_stories_react(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_react(kb_client *c, kb_stories_react_response *resp);

kb_status kb_send_stories_mark(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_mark(kb_client *c, kb_stories_mark_response *resp);

kb_status kb_send_stories_send(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_send(kb_client *c, kb_stories_send_response *resp);

kb_status kb_send_notif_stories_update(kb_session *s, uint16_t *seq);
kb_status kb_call_notif_stories_update(kb_client *c, kb_notif_stories_update_response *resp);

kb_status kb_send_stories_edit(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_edit(kb_client *c, kb_stories_edit_response *resp);

kb_status kb_send_stories_delete(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_delete(kb_client *c, kb_stories_delete_response *resp);

kb_status kb_send_stories_get_by_story_id(kb_session *s, uint16_t *seq);
kb_status kb_call_stories_get_by_story_id(kb_client *c, kb_stories_get_by_story_id_response *resp);

kb_status kb_send_org_info(kb_session *s, const kb_org_info *v, uint16_t *seq);
kb_status kb_call_org_info(kb_client *c, const kb_org_info *v, kb_org_info_response *resp);

kb_status kb_send_chat_reactions_settings_set(kb_session *s, uint16_t *seq);
kb_status kb_call_chat_reactions_settings_set(kb_client *c, kb_chat_reactions_settings_set_response *resp);

kb_status kb_send_reactions_settings_get_by_chat_id(kb_session *s, uint16_t *seq);
kb_status kb_call_reactions_settings_get_by_chat_id(kb_client *c, kb_reactions_settings_get_by_chat_id_response *resp);

kb_status kb_send_folders_get(kb_session *s, const kb_folders_get *v, uint16_t *seq);
kb_status kb_call_folders_get(kb_client *c, const kb_folders_get *v, kb_folders_get_response *resp);

kb_status kb_send_folders_get_by_id(kb_session *s, uint16_t *seq);
kb_status kb_call_folders_get_by_id(kb_client *c, kb_folders_get_by_id_response *resp);

kb_status kb_send_folders_update(kb_session *s, const kb_folders_update *v, uint16_t *seq);
kb_status kb_call_folders_update(kb_client *c, const kb_folders_update *v, kb_folders_update_response *resp);

kb_status kb_send_folders_reorder(kb_session *s, const kb_folders_reorder *v, uint16_t *seq);
kb_status kb_call_folders_reorder(kb_client *c, const kb_folders_reorder *v, kb_folders_reorder_response *resp);

kb_status kb_send_folders_delete(kb_session *s, const kb_folders_delete *v, uint16_t *seq);
kb_status kb_call_folders_delete(kb_client *c, const kb_folders_delete *v, kb_folders_delete_response *resp);

kb_status kb_send_notif_folders(kb_session *s, uint16_t *seq);
kb_status kb_call_notif_folders(kb_client *c, kb_notif_folders_response *resp);

kb_status kb_send_transcription_result(kb_session *s, uint16_t *seq);
kb_status kb_call_transcription_result(kb_client *c, kb_transcription_result_response *resp);

kb_status kb_send_chat_suggest(kb_session *s, const kb_chat_suggest *v, uint16_t *seq);
kb_status kb_call_chat_suggest(kb_client *c, const kb_chat_suggest *v, kb_chat_suggest_response *resp);

kb_status kb_send_banners_get(kb_session *s, const kb_banners_get *v, uint16_t *seq);
kb_status kb_call_banners_get(kb_client *c, const kb_banners_get *v, kb_banners_get_response *resp);

kb_status kb_send_send_vote(kb_session *s, const kb_send_vote *v, uint16_t *seq);
kb_status kb_call_send_vote(kb_client *c, const kb_send_vote *v, kb_send_vote_response *resp);

kb_status kb_send_voters_list_by_answer(kb_session *s, const kb_voters_list_by_answer *v, uint16_t *seq);
kb_status kb_call_voters_list_by_answer(kb_client *c, const kb_voters_list_by_answer *v, kb_voters_list_by_answer_response *resp);

kb_status kb_send_get_poll_updates(kb_session *s, const kb_get_poll_updates *v, uint16_t *seq);
kb_status kb_call_get_poll_updates(kb_client *c, const kb_get_poll_updates *v, kb_get_poll_updates_response *resp);

#if defined(__cplusplus)
}
#endif

#endif /* KOLIBRI_API_H */
