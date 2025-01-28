
#ifndef MAIN_H
#define MAIN_H

#include <steam_api.h> 

class Callback_Handler{
    public:
        char *dir_path = NULL;
        AppId_t app_id = NULL;
        void on_workshop_item_created(CreateItemResult_t *pCallback, bool bIOFailure);
        void on_workshop_item_updated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure);
};

bool update_mod(const AppId_t app_id, const PublishedFileId_t mod_id, char *dir_path);

#endif // MAIN_H

