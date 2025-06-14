/*
 * Credits: silviu20092
 */

#include "Player.h"
#include "Creature.h"
#include "ScriptedGossip.h"
#include "StringConvert.h"
#include "Group.h"
#include "mythic_plus.h"
#include "mythic_affix.h"
#include "mythic_plus_npc_support.h"

void MythicPlusNpcSupport::AddMainMenu(Player* player, Creature* creature)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.backMenu = false;
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_NPC_MENU;

    if (!sMythicPlus->IsEnabled())
    {
        Identifier* disabledIdnt = new Identifier();
        disabledIdnt->id = 0;
        disabledIdnt->uiName = MythicPlus::Utils::RedColored("!!! SYSTEM IS NOT ACTIVE !!!");
        pagedData.data.push_back(disabledIdnt);
    }

    Identifier* i1 = new Identifier();
    i1->id = 1;
    i1->uiName = "Choose Mythic Plus level";
    i1->optionIcon = GOSSIP_ICON_BATTLE;
    pagedData.data.push_back(i1);

    uint32 setLevel = sMythicPlus->GetCurrentMythicPlusLevel(player);
    if (setLevel > 0)
    {
        Identifier* resetIdnt = new Identifier();
        resetIdnt->id = 2;
        resetIdnt->uiName = "Reset Mythic Plus level [CURRENT: " + Acore::ToString(setLevel) + "]";
        resetIdnt->optionIcon = GOSSIP_ICON_BATTLE;
        pagedData.data.push_back(resetIdnt);
    }
    else
    {
        Identifier* nothingIdnt = new Identifier();
        nothingIdnt->id = 3;
        nothingIdnt->uiName = MythicPlus::Utils::Colored("No Mythic Plus level set", "b50505");
        nothingIdnt->optionIcon = GOSSIP_ICON_CHAT;
        pagedData.data.push_back(nothingIdnt);
    }

    if (player->GetGroup() != nullptr)
    {
        ObjectGuid leaderGuid = player->GetGroup()->GetLeaderGUID();
        uint32 leaderLevel = sMythicPlus->GetCurrentMythicPlusLevelForGUID(leaderGuid.GetCounter());
        Player* leader = ObjectAccessor::FindConnectedPlayer(leaderGuid);
        Identifier* dungeonLevelIdnt = new Identifier();
        dungeonLevelIdnt->id = 4;
        dungeonLevelIdnt->optionIcon = GOSSIP_ICON_CHAT;
        std::ostringstream oss;
        oss << "Current dungeon Mythic Plus level (based on leader): ";
        if (leaderLevel == 0)

            oss << MythicPlus::Utils::Colored("NONE (0)", "b50505");
        else
            oss << leaderLevel;
        if (!leader)
            oss << " [LEADER OFFLINE]";
        dungeonLevelIdnt->uiName = oss.str();
        pagedData.data.push_back(dungeonLevelIdnt);
    }

    Identifier* mPlusListIdnt = new Identifier();
    mPlusListIdnt->id = 5;
    mPlusListIdnt->uiName = "List of all Mythic Plus capable dungeons";
    pagedData.data.push_back(mPlusListIdnt);

    Identifier* standingsRefreshIdnt = new Identifier();
    standingsRefreshIdnt->id = 6;
    std::ostringstream oss;
    oss << "Mythic Plus standings refresh in: ";
    oss << MythicPlus::Utils::Colored(secsToTimeString((MythicPlus::MYTHIC_SNAPSHOTS_TIMER_FREQ - sMythicPlus->GetMythicSnapshotsTimer()) / 1000), "b50505");
    standingsRefreshIdnt->uiName = oss.str();
    pagedData.data.push_back(standingsRefreshIdnt);

    Identifier* standings = new Identifier();
    standings->id = 7;
    standings->uiName = "Mythic Plus standings -->";
    pagedData.data.push_back(standings);

    Identifier* bye = new Identifier();
    bye->id = 8;
    bye->uiName = "Nevermind...";
    pagedData.data.push_back(bye);

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusLevels(Player* player, Creature* creature)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVELS;

    const MythicLevelContainer& mythicLevels = sMythicPlus->GetAllMythicLevels();
    for (const auto& mlevel : mythicLevels)
    {
        Identifier* idnt = new Identifier();
        idnt->id = mlevel.level;
        idnt->optionIcon = GOSSIP_ICON_BATTLE;
        std::ostringstream oss;
        oss << "Mythic level " << mlevel.level;
        oss << " (" << mlevel.affixes.size() << " affix(es)) -->";
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusLevelInfo(Player* player, Creature* creature, uint32 mythicLevel)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVEL_INFO;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mythicLevel = mythicLevel;

    uint32 id = 0;

    Identifier* idnt = new Identifier();
    idnt->id = ++id;
    idnt->optionIcon = GOSSIP_ICON_BATTLE;
    idnt->uiName = "Click to choose Mythic Level " + Acore::ToString(mythicLevel);
    pagedData.data.push_back(idnt);

    const MythicLevel* level = sMythicPlus->GetMythicLevel(mythicLevel);
    ASSERT(level);

    Identifier* timerIdnt = new Identifier();
    timerIdnt->id = ++id;
    timerIdnt->uiName = "Time limit to get rewards: " + secsToTimeString(level->timeLimit);
    pagedData.data.push_back(timerIdnt);

    for (int i = 0; i < level->affixes.size(); i++)
    {
        Identifier* affixIdnt = new Identifier();
        affixIdnt->id = ++id;
        std::ostringstream oss;
        oss << "Affix ";
        oss << i + 1 << ": ";
        oss << level->affixes.at(i)->ToString();
        affixIdnt->uiName = oss.str();
        pagedData.data.push_back(affixIdnt);
    }

    Identifier* rewardsIdnt = new Identifier();
    rewardsIdnt->id = ++id;
    rewardsIdnt->optionIcon = GOSSIP_ICON_MONEY_BAG;
    rewardsIdnt->uiName = MythicPlus::Utils::Colored("-- REWARDS --", "0d852d");
    pagedData.data.push_back(rewardsIdnt);

    const MythicReward& reward = level->reward;
    if (reward.money)
    {
        Identifier* moneyIdnt = new Identifier();
        moneyIdnt->id = ++id;
        moneyIdnt->optionIcon = GOSSIP_ICON_MONEY_BAG;
        moneyIdnt->uiName = "Gold: " + MythicPlus::Utils::CopperToMoneyStr(reward.money, false);
        pagedData.data.push_back(moneyIdnt);
    }

    if (!reward.tokens.empty())
    {
        for (const auto& token : reward.tokens)
        {
            Identifier* tokenIdnt = new Identifier();
            tokenIdnt->id = ++id;
            tokenIdnt->optionIcon = GOSSIP_ICON_VENDOR;
            std::ostringstream oss;
            oss << MythicPlus::Utils::ItemLinkForUI(token.first, player);
            oss << " - " << token.second << "x";
            tokenIdnt->uiName = oss.str();
            pagedData.data.push_back(tokenIdnt);
        }
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusDungeonList(Player* player, Creature* creature)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST;

    const std::unordered_map<uint32, Difficulty>& dungeons = sMythicPlus->GetAllMythicPlusDungeons();
    LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
    uint32 id = 0;
    for (const auto& dpair : dungeons)
    {
        uint32 mapEntry = dpair.first;
        Difficulty diff = dpair.second;

        MapEntry const* map = sMapStore.LookupEntry(mapEntry);
        ASSERT(map);

        Identifier* idnt = new Identifier();
        idnt->id = ++id;
        std::ostringstream oss;
        oss << map->name[locale];
        oss << " [";
        if (diff == DUNGEON_DIFFICULTY_NORMAL)
            oss << "NORMAL/HEROIC]";
        else
            oss << "HEROIC ONLY]";
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusDungeonListForSnapshots(Player* player, Creature* creature, uint32 snapMythicLevel)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel = snapMythicLevel;

    Identifier* mlevelIdnt = new Identifier();
    mlevelIdnt->id = 1;
    std::ostringstream oss;
    oss << "Selected Mythic Plus level: ";
    if (snapMythicLevel == 0)
        oss << "ALL";
    else
        oss << snapMythicLevel;
    mlevelIdnt->uiName = oss.str();
    mlevelIdnt->optionIcon = GOSSIP_ICON_CHAT;
    pagedData.data.push_back(mlevelIdnt);

    const std::unordered_map<uint32, Difficulty>& dungeons = sMythicPlus->GetAllMythicPlusDungeons();
    LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();
    for (const auto& dpair : dungeons)
    {
        uint32 mapEntry = dpair.first;

        MapEntry const* map = sMapStore.LookupEntry(mapEntry);
        ASSERT(map);

        const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>> snapshots = sMythicPlus->GetMapSnapshot(mapEntry, snapMythicLevel);

        Identifier* idnt = new Identifier();
        idnt->id = mapEntry;
        std::ostringstream oss;
        oss << map->name[locale];
        oss << " [TOTAL RUNS: ";
        if (snapshots.empty())
            oss << MythicPlus::Utils::Colored("NONE", "b50505");
        else
            oss << snapshots.size();
        oss << "]";
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusSnapshotAllRuns(Player* player, Creature* creature, uint32 mapEntry)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT_RUNS;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mapEntry = mapEntry;

    MapEntry const* map = sMapStore.LookupEntry(mapEntry);
    ASSERT(map);

    LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();

    Identifier* mapIdnt = new Identifier();
    mapIdnt->id = 1;
    std::ostringstream oss;
    oss << "Mythic Plus top timers for ";
    oss << map->name[locale];
    mapIdnt->uiName = oss.str();
    pagedData.data.push_back(mapIdnt);

    uint32 snapMythicLevel = pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel;

    const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>> snapshots = sMythicPlus->GetMapSnapshot(mapEntry, snapMythicLevel);
    if (!snapshots.empty())
    {
        for (const auto& s : snapshots)
        {
            const MythicPlus::MythicPlusDungeonSnapshot& snap = s.second.at(0);
            uint32 internalId = snap.internalId;

            Identifier* idnt = new Identifier();
            idnt->id = internalId + 10;
            std::ostringstream oss;
            oss << internalId << ". ";
            const MythicLevel* mlevel = sMythicPlus->GetMythicLevel(snap.mythicLevel);
            ASSERT(mlevel);
            if (snap.totalTime > 0)
            {
                oss << secsToTimeString(snap.totalTime);
                oss << " [LIMIT: ";
                oss << secsToTimeString(mlevel->timeLimit);
                oss << "]";
                if (snap.rewarded)
                    oss << MythicPlus::Utils::GreenColored(" [REWARDED]");
                else
                    oss << MythicPlus::Utils::RedColored(" [NOT REWARDED]");
            }
            else
            {
                oss << MythicPlus::Utils::RedColored("NOT FINISHED");
                oss << " [LIMIT: ";
                oss << secsToTimeString(mlevel->timeLimit);
                oss << "]";
            }
            oss << " [M+ LEVEL ";
            oss << snap.mythicLevel;
            oss << "]";
            idnt->uiName = oss.str();
            pagedData.data.push_back(idnt);
        }
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusAllLevels(Player* player, Creature* creature)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_ALL_LEVELS;

    Identifier* allIdnt = new Identifier();
    allIdnt->id = 0;
    allIdnt->uiName = "ALL Mythic Plus levels";
    pagedData.data.push_back(allIdnt);

    const MythicLevelContainer& mythicLevels = sMythicPlus->GetAllMythicLevels();
    for (const auto& mlevel : mythicLevels)
    {
        Identifier* idnt = new Identifier();
        idnt->id = mlevel.level;
        idnt->optionIcon = GOSSIP_ICON_BATTLE;
        std::ostringstream oss;
        oss << "Mythic level " << mlevel.level << " -- >";
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

void MythicPlusNpcSupport::AddMythicPlusDungeonSnapshotDetails(Player* player, Creature* creature, uint32 internalId)
{
    PagedData& pagedData = GetPagedData(player);
    pagedData.Reset();
    pagedData.type = GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_SNAPSHOT_DETAILS;
    pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->internalId = internalId;

    uint32 mapEntry = pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mapEntry;
    uint32 snapMythicLevel = pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel;
    const std::vector<std::pair<std::pair<uint32, uint64>, std::vector<MythicPlus::MythicPlusDungeonSnapshot>>> snapshots = sMythicPlus->GetMapSnapshot(mapEntry, snapMythicLevel);

    std::vector<MythicPlus::MythicPlusDungeonSnapshot> chosenSnaps;
    for (const auto& s : snapshots)
    {
        if (s.second.at(0).internalId == internalId)
        {
            chosenSnaps = s.second;
            break;
        }
    }
    if (chosenSnaps.empty())
        return;

    std::sort(chosenSnaps.begin(), chosenSnaps.end(), [](const MythicPlus::MythicPlusDungeonSnapshot& a, const MythicPlus::MythicPlusDungeonSnapshot& b) {
        return a.snapTime < b.snapTime;
    });

    MapEntry const* map = sMapStore.LookupEntry(mapEntry);
    ASSERT(map);

    LocaleConstant locale = player->GetSession()->GetSessionDbcLocale();

    const MythicPlus::MythicPlusDungeonSnapshot* csnap = &chosenSnaps.at(0);

    Identifier* idnt = new Identifier();
    idnt->id = 1;
    std::ostringstream oss;
    oss << "Mythic Plus run for ";
    oss << map->name[locale];
    oss << " at level ";
    oss << csnap->mythicLevel;
    idnt->uiName = oss.str();
    pagedData.data.push_back(idnt);

    oss.str("");
    oss.clear();

    Identifier* startTimeIdnt = new Identifier();
    startTimeIdnt->id = 2;
    oss << "Run started at ";
    oss << MythicPlus::Utils::DateFromSeconds(csnap->startTime);
    oss << " [UTC TIME]";
    startTimeIdnt->uiName = oss.str();
    pagedData.data.push_back(startTimeIdnt);

    oss.str("");
    oss.clear();

    Identifier* endTimeIdnt = new Identifier();
    endTimeIdnt->id = 3;
    if (csnap->totalTime > 0)
    {
        oss << "Run ended at ";
        oss << MythicPlus::Utils::DateFromSeconds(csnap->endTime);
        oss << " [UTC TIME]";
        oss << " [DURATION: ";
        oss << secsToTimeString(csnap->totalTime);
        oss << "]";
    }
    else
        oss << MythicPlus::Utils::RedColored("Run did not end (yet, or orphaned instance)");
    endTimeIdnt->uiName = oss.str();
    pagedData.data.push_back(endTimeIdnt);

    uint32 id = 3;
    for (const auto& s : chosenSnaps)
    {
        oss.str("");
        oss.clear();

        oss << MythicPlus::Utils::Colored(MythicPlus::Utils::GetCreatureNameByEntry(player, s.entry), "102163");
        oss << " downed at ";
        oss << MythicPlus::Utils::DateFromSeconds(s.snapTime);
        oss << " [took ";
        oss << secsToTimeString(s.combatTime);
        oss << "]";
        oss << " [PLAYERS: ";
        oss << MythicPlus::Utils::Colored(s.players, "6e1849");
        oss << "]";

        Identifier* idnt = new Identifier();
        idnt->id = ++id;
        idnt->uiName = oss.str();
        pagedData.data.push_back(idnt);
    }

    if (csnap->totalTime > 0)
    {
        Identifier* rewardIdnt = new Identifier();
        rewardIdnt->id = ++id;
        if (csnap->rewarded)
            rewardIdnt->uiName = MythicPlus::Utils::GreenColored("TIMER WAS BEATEN - REWARDS RECEIVED");
        else
            rewardIdnt->uiName = MythicPlus::Utils::RedColored("NO REWARDS RECEIVED - TIMER LIMIT EXCEEDED");
        pagedData.data.push_back(rewardIdnt);
    }

    pagedData.SortAndCalculateTotals(CompareIdentifierById);
}

bool MythicPlusNpcSupport::TakePagedDataAction(Player* player, Creature* creature, uint32 action)
{
    PagedData& pagedData = GetPagedData(player);
    if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_NPC_MENU)
    {
        if (action == 1)
        {
            AddMythicPlusLevels(player, creature);
            return AddPagedData(player, creature, 0);
        }
        else if (action == 2)
        {
            if (sMythicPlus->SetCurrentMythicPlusLevel(player, 0))
            {
                MythicPlus::BroadcastToPlayer(player, "Your Mythic Plus level was reset!");
                MythicPlus::Utils::VisualFeedback(player);
            }
            else
                MythicPlus::BroadcastToPlayer(player, "You can't reset your Mythic Plus level while in a group.");

            CloseGossipMenuFor(player);
            return true;
        }
        else if (action == 3 || action == 4 || action == 6 || action == 0)
            return OnGossipHello(player, creature);
        else if (action == 5)
        {
            AddMythicPlusDungeonList(player, creature);
            return AddPagedData(player, creature, 0);
        }
        else if (action == 7)
        {
            AddMythicPlusAllLevels(player, creature);
            return AddPagedData(player, creature, 0);
        }
        else if (action == 8)
        {
            CloseGossipMenuFor(player);
            return true;
        }
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVELS)
    {
        AddMythicPlusLevelInfo(player, creature, action);
        return AddPagedData(player, creature, 0);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVEL_INFO)
    {
        uint32 chosenMythicLevel = pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mythicLevel;
        if (action == 1)
        {
            if (sMythicPlus->SetCurrentMythicPlusLevel(player, chosenMythicLevel))
            {
                MythicPlus::BroadcastToPlayer(player, "Your Mythic Plus level was set to " + Acore::ToString(chosenMythicLevel));
                MythicPlus::Utils::VisualFeedback(player);
            }
            else
                MythicPlus::BroadcastToPlayer(player, "You can't set your Mythic Plus level while in a group.");

            CloseGossipMenuFor(player);
            return true;
        }
        else
        {
            AddMythicPlusLevelInfo(player, creature, chosenMythicLevel);
            return AddPagedData(player, creature, pagedData.currentPage);
        }
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST)
    {
        AddMythicPlusDungeonList(player, creature);
        return AddPagedData(player, creature, pagedData.currentPage);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT)
    {
        if (action == 1)
        {
            AddMythicPlusDungeonListForSnapshots(player, creature, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel);
            return AddPagedData(player, creature, pagedData.currentPage);
        }
        AddMythicPlusSnapshotAllRuns(player, creature, action);
        return AddPagedData(player, creature, 0);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_ALL_LEVELS)
    {
        AddMythicPlusDungeonListForSnapshots(player, creature, action);
        return AddPagedData(player, creature, 0);
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT_RUNS)
    {
        if (action == 1)
        {
            AddMythicPlusSnapshotAllRuns(player, creature, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mapEntry);
            return AddPagedData(player, creature, pagedData.currentPage);
        }
        else
        {
            AddMythicPlusDungeonSnapshotDetails(player, creature, action - 10);
            return AddPagedData(player, creature, 0);
        }
    }
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_SNAPSHOT_DETAILS)
    {
        AddMythicPlusDungeonSnapshotDetails(player, creature, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->internalId);
        return AddPagedData(player, creature, pagedData.currentPage);
    }

    return GossipSupport::TakePagedDataAction(player, creature, action);
}

/*static*/ bool MythicPlusNpcSupport::CompareIdentifierById(const Identifier* a, const Identifier* b)
{
    return a->id < b->id;
}

uint32 MythicPlusNpcSupport::_PageZeroSender(const PagedData& pagedData) const
{
    if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVELS
        || pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST
        || pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_ALL_LEVELS)
        return GOSSIP_SENDER_MAIN;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_LEVEL_INFO)
        return GOSSIP_SENDER_MAIN + 9;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT)
        return GOSSIP_SENDER_MAIN + 10;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_LIST_SNAPSHOT_RUNS)
        return GOSSIP_SENDER_MAIN + 11;
    else if (pagedData.type == GossipSupport::PAGED_DATA_TYPE_MYTHIC_DUNGEON_SNAPSHOT_DETAILS)
        return GOSSIP_SENDER_MAIN + 12;

    return GOSSIP_SENDER_MAIN;
}

bool MythicPlusNpcSupport::OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
{
    PagedData& pagedData = GetPagedData(player);
    if (sender <= GOSSIP_SENDER_MAIN + 2)
        return GossipSupport::OnGossipSelect(player, creature, sender, action);
    else if (sender == GOSSIP_SENDER_MAIN + 9)
    {
        AddMythicPlusLevels(player, creature);
        return AddPagedData(player, creature, 0);
    }
    else if (sender == GOSSIP_SENDER_MAIN + 10)
    {
        AddMythicPlusAllLevels(player, creature);
        return AddPagedData(player, creature, 0);
    }
    else if (sender == GOSSIP_SENDER_MAIN + 11)
    {
        AddMythicPlusDungeonListForSnapshots(player, creature, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->snapMythicLevel);
        return AddPagedData(player, creature, 0);
    }
    else if (sender == GOSSIP_SENDER_MAIN + 12)
    {
        AddMythicPlusSnapshotAllRuns(player, creature, pagedData.GetCustomInfo<MythicPlusNpcPageInfo>()->mapEntry);
        return AddPagedData(player, creature, 0);
    }

    return false;
}
