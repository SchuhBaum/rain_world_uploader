#include <filesystem>
#include <fstream>
#include <iostream>
namespace fs = std::filesystem;

#include <chrono>
#include <thread>

#include "steam_api.h"

#include "main.h"

//
//
//

Callback_Handler callback_handler;
bool is_done = false;

//
// class Callback_Handler
//

void Callback_Handler::on_workshop_item_created(CreateItemResult_t *pCallback, bool bIOFailure) {
    if (bIOFailure) {
        fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Creating the mod failed due to an IO error!\n");
        is_done = true;
        return;
    }

    if (pCallback->m_eResult == k_EResultOK) {
        printf("[Rain_World_Uploader]: A new mod with MOD_ID %llu was created successfully!\n", pCallback->m_nPublishedFileId);
    } else {
        fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Failed to create item. Error: %d\n", pCallback->m_eResult);
    }

    if (this->app_id != NULL && this->dir_path != NULL) {
        PublishedFileId_t mod_id = pCallback->m_nPublishedFileId;
        if (update_mod(app_id, mod_id, this->dir_path)) {
            is_done = false;
            return;
        }
    }
    is_done = true;
}

void Callback_Handler::on_workshop_item_updated(SubmitItemUpdateResult_t *pCallback, bool bIOFailure) {
    if (bIOFailure) {
        fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Updating the mod failed due to an IO error!\n");
        is_done = true;
        return;
    }

    if (pCallback->m_eResult == k_EResultOK) {
        printf("[Rain_World_Uploader]: Mod updated successfully!\n");
    } else {
        fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Failed to update the mod. Error: %d\n", pCallback->m_eResult);
    }

    is_done = true;
}

//
// utility functions
//

bool create_mod(const AppId_t app_id, char *dir_path) {
    // Check the path first. We update the mod after it is created and upload
    // the mod directory.
    if (!fs::is_directory(dir_path)) {
        fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Did not find the mod directory at \"%s\".\n", dir_path);
        return false;
    }

    callback_handler.app_id   = app_id;
    callback_handler.dir_path = dir_path;
    
    printf("\n");
    printf("[Rain_World_Uploader]: Create a new mod item.\n");

    // Static means that it is global but only initialized once.
    SteamAPICall_t callback_handle = SteamUGC()->CreateItem(app_id, k_EWorkshopFileTypeCommunity);
    static CCallResult<Callback_Handler, CreateItemResult_t> call_result;
    call_result.Set(callback_handle, &callback_handler, &Callback_Handler::on_workshop_item_created);

    printf("[Rain_World_Uploader]: Waiting for results...\n");
    return true;
}

bool update_mod(const AppId_t app_id, PublishedFileId_t mod_id, char *dir_path) {
    if (!fs::is_directory(dir_path)) {
        fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Did not find the mod directory at \"%s\".\n", dir_path);
        return false;
    }

    printf("\n");
    printf("[Rain_World_Uploader]: Update the mod with MOD_ID %lld.\n", mod_id);
    UGCUpdateHandle_t update_handle = SteamUGC()->StartItemUpdate(app_id, mod_id);

    const fs::path raw_absolute_dir_path = fs::absolute(fs::canonical(fs::path(dir_path)));
    if (!SteamUGC()->SetItemContent(update_handle, raw_absolute_dir_path.string().c_str())) return false;

    // Sets the change note to "". It can be changed in Steam ([Select the
    // Workshop item] > Change Notes > Edit).
    SteamAPICall_t callback_handle = SteamUGC()->SubmitItemUpdate(update_handle, "");
    static CCallResult<Callback_Handler, SubmitItemUpdateResult_t> call_result;
    call_result.Set(callback_handle, &callback_handler, &Callback_Handler::on_workshop_item_updated);

    printf("[Rain_World_Uploader]: Waiting for results...\n");
    return true;
}

bool set_thumbnail(const AppId_t app_id, const PublishedFileId_t mod_id, char *file_path) {
    if (!fs::exists(file_path)) {
        fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Did not find the thumbnail at \"%s\".\n", file_path);
        return false;
    }

    printf("\n");
    printf("[Rain_World_Uploader]: Setting the thumbnail for the mod with MOD_ID %lld.\n", mod_id);

    UGCUpdateHandle_t update_handle       = SteamUGC()->StartItemUpdate(app_id, mod_id);
    const fs::path raw_absolute_file_path = fs::absolute(fs::canonical(fs::path(file_path)));
    if (!SteamUGC()->SetItemPreview(update_handle, raw_absolute_file_path.string().c_str())) return false;

    SteamAPICall_t callback_handle = SteamUGC()->SubmitItemUpdate(update_handle, "");
    static CCallResult<Callback_Handler, SubmitItemUpdateResult_t> call_result;
    call_result.Set(callback_handle, &callback_handler, &Callback_Handler::on_workshop_item_updated);

    printf("[Rain_World_Uploader]: Waiting for results...\n");
    return true;
}

bool set_tags(const AppId_t app_id, const PublishedFileId_t mod_id, const int tagc, const char **tagv) {
    printf("\n");
    printf("[Rain_World_Uploader]: Setting the tags for the mod with MOD_ID %lld to", mod_id);
    for (int i = 0; i < tagc; ++i) {
        printf(" \"%s\"", tagv[i]);
    }
    printf(".\n");

    SteamParamStringArray_t tag_array;
    tag_array.m_ppStrings   = tagv;
    tag_array.m_nNumStrings = tagc;

    UGCUpdateHandle_t update_handle = SteamUGC()->StartItemUpdate(app_id, mod_id);
    if (!SteamUGC()->SetItemTags(update_handle, &tag_array)) return false;

    SteamAPICall_t callback_handle = SteamUGC()->SubmitItemUpdate(update_handle, "");
    static CCallResult<Callback_Handler, SubmitItemUpdateResult_t> call_result;
    call_result.Set(callback_handle, &callback_handler, &Callback_Handler::on_workshop_item_updated);

    printf("[Rain_World_Uploader]: Waiting for results...\n");
    return true;
}

void print_usage() {
    printf("\n");
    printf("[USAGE]: Create a mod:\n    $ rain_world_uploader.out /path/to/mod/directory\n    NOTE: This returns the number MOD_ID for the created mod.\n");
    printf("[USAGE]: Update a mod:\n    $ rain_world_uploader.out MOD_ID /path/to/mod/directory\n    NOTE: You get the MOD_ID from creating a mod, or from a Workshop URL ([...]?id=MOD_ID).\n");
    printf("[USAGE]: Set thumbnail:\n    $ rain_world_uploader.out MOD_ID -thumbnail /path/to/thumbnail.png\n");
    printf("[USAGE]: Set tags (this overrides existing tags):\n    $ rain_world_uploader.out MOD_ID -tags \"Tag 1\" \"Tag 2\" ...\n");
}

AppId_t get_app_id_from_file() {
    AppId_t app_id = 312520;

    std::ifstream file("steam_appid.txt");

    if (!file) return app_id;

    std::string app_id_str;
    std::getline(file, app_id_str);
    file.close();

    if (app_id_str.empty()) return app_id;

    try {
        AppId_t parsed_app_id = static_cast<AppId_t>(std::stoul(app_id_str));
        return parsed_app_id;
    } catch (...) { }

    return app_id;
}

//
// main
//

int main(int argc, char **argv) {
    // The function SteamAPI_Init() looks for the file `steam_appid.txt`. Since
    // this is the case I can just extract the app_id from there as well.
    if (!SteamAPI_Init()) {
        fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Failed to initialize Steamworks! Is Steam running?\n");
        SteamAPI_Shutdown();
        return 1;
    }

    printf("\n");
    AppId_t app_id = get_app_id_from_file();
    printf("[Rain_World_Uploader]: The APP_ID is %d.\n", app_id);

    printf("[Rain_World_Uploader]: Executing the command:\n    ");
    for (int i = 0; i < argc; ++i) printf("%s ", argv[i]);
    printf("\n");

    // Parse arguments and setup Steam async callback functions. They will be
    // called when the mod is created or updated successfully, or when it
    // failed.
    if (argc < 2) {
        print_usage();
        return 1;

    } else if (argc == 2) {
        char *dir_path = argv[1];
        if (!create_mod(app_id, dir_path)) return 1;

    } else if (argc >= 3) {
        PublishedFileId_t mod_id;
        std::stringstream ss(argv[1]);
        ss >> mod_id;

        if (ss.fail()) {
            print_usage();
            return 1;
        }

        char *argv_2 = argv[2];
        if (argv_2[0] == '-') {
            if (strcmp(argv_2, "-thumbnail") != 0 && strcmp(argv_2, "-tags") != 0) {
                fprintf(stderr, "[Rain_World_Uploader]: [ERROR] Expected \"-thumbnail\" or \"-tags\" as third argument.\n");
                print_usage();
                return 1;

            } else if (strcmp(argv_2, "-thumbnail") == 0) {
                char *file_path = argv[3];
                if (!set_thumbnail(app_id, mod_id, file_path)) return 1;

            } else if (strcmp(argv_2, "-tags") == 0) {
                if (!set_tags(app_id, mod_id, argc-3, (const char **)argv+3)) return 1;

            } else {
                print_usage();
                return 1;
            }

        } else {
            char *dir_path = argv_2;
            if (!update_mod(app_id, mod_id, dir_path)) return 1;
        }
    }

    while (!is_done) {
        SteamAPI_RunCallbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    SteamAPI_Shutdown();
    return 0;
}

