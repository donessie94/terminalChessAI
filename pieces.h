#include<vector>
#include<unordered_map>

struct PIECES
{
    // Rook design 6 lines × 11 columns.
    std::vector<std::string> rookArt = {
        "  _  _  _  ",
        " | || || | ",
        " |_______| ",
        " \\__ ___ / ",
        " _|___|_|_ ",
        "(__|______)"
    };

    // Knight design
    std::vector<std::string> knightArt = {
        "    (\\=,   ",
        "  //  .\\   ",
        " (( \\_  \\  ",
        "  ))  `\\_) ",
        " (/_____\\_ ",
        "(_________)"
    };

    // Bishop design
    std::vector<std::string> bishopArt = {
        "    _O     ",
        "   / //\\   ",
        "  {     }  ",
        "   \\___/   ",
        " __(___)__ ",
        "(_________)"
    };

    // Queen design
    std::vector<std::string> queenArt = {
        "  .-:--:-. ",
        "   \\_\\_//  ",
        "   {====}  ",
        "    )__(   ",
        "  __)__(__ ",
        "(_________)"
    };

    // King design
    std::vector<std::string> kingArt = {
        "    .:.    ",
        "   _.+._   ",
        " (^\\/^\\/^) ",
        "  \\@*@*@/  ",
        " _{_____}_ ",
        "(_________)"
    };

    // Pawn design
    std::vector<std::string> pawnArt = {
        "     _     ",
        "    (_)    ",
        "   (___)   ",
        "   _|_|_   ",
        "  (_____)  ",
        "           "
    };

    // Empty design
    std::vector<std::string> emptyArt = {
        "           ",
        "           ",
        "           ",
        "           ",
        "           ",
        "           "
    };

    int colSize=11;
    int rowSize=6;
};

/*
PIECE DESIGNS
  _  _  _      (\=,         _O       .-:--:-.      .:.
 | || || |    //  .\       / //\      \_\_//      _.+._
 |_______|   (( \_  \     {     }     {====}    (^\/^\/^)
 \__ ___ /    ))  `\_)     \___/       )__(      \@*@*@/
 _|___|_|_   (/_____\_   __(___)__   __)__(__   _{_____}_
(__|______) (_________) (_________) (________) (_________)
    _
   (_)
  (___)
  _|_|_
 (_____)
*/