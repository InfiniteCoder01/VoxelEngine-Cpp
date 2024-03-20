#include "menu.h"
#include "menu_commons.h"
#include "../gui/controls.h"
#include "../gui/gui_util.h"
#include "../locale/langs.h"
#include "../screens.h"
#include "../../logic/LevelController.h"
#include "../../world/WorldGenerators.h"
#include "../../world/World.h"
#include "../../world/Level.h"
#include "../../util/stringutil.h"
#include "../../engine.h"

using namespace gui;

void menus::create_join_server_panel(Engine* engine) {
    auto panel = menus::create_page(engine, "join-server", 400, 0.0f, 1);

    panel->add(std::make_shared<Label>(langs::get(L"Address", L"server")));
    auto addressInput = std::make_shared<TextBox>(L"localhost:1234", glm::vec4(6.0f));
    // addressInput->setTextValidator([=](const std::wstring& text) {
    //     EnginePaths* paths = engine->getPaths();
    //     std::string textutf8 = util::wstr2str_utf8(text);
    //     return util::is_valid_filename(text) && 
    //             !paths->isWorldNameUsed(textutf8);
    // });
    panel->add(addressInput);

    panel->add(std::make_shared<Label>(langs::get(L"Join as (Name)", L"server")));
    auto nameInput = std::make_shared<TextBox>(langs::get(L"Guest", L"server"), glm::vec4(6.0f));
    panel->add(nameInput);
    
    panel->add(menus::create_button(L"Join Server", glm::vec4(10), glm::vec4(1, 20, 1, 1), 
    [=](GUI*) {
        // if (!addressInput->validate())
        //     return;
        // if (!nameInput->validate())
        //     return;

        std::string address = util::wstr2str_utf8(addressInput->getText());
        std::string name = util::wstr2str_utf8(nameInput->getText());

        try {
            auto client = std::make_unique<ClientController>(engine, address, name);
            auto screen = std::make_shared<LevelScreen>(engine, client->level);
            screen->getLevelController()->client = std::move(client);
            engine->setScreen(screen);
        } catch (const std::wstring& error) {
            guiutil::alert(engine->getGUI(), error);
            return;
        } catch (const contentpack_error& error) {
            guiutil::alert(
                engine->getGUI(),
                langs::get(L"Content Error", L"menu")+L":\n"+
                util::str2wstr_utf8(
                    std::string(error.what())+
                    "\npack '"+error.getPackId()+"' from "+
                    error.getFolder().u8string()
                )
            );
            return;
        } catch (const std::runtime_error& error) {
            guiutil::alert(
                engine->getGUI(),
                langs::get(L"Content Error", L"menu")+
                L": "+util::str2wstr_utf8(error.what())
            );
            return;
        }
    }));
    panel->add(guiutil::backButton(engine->getGUI()->getMenu()));
}
