
// StatSerializer.cpp


#include "Globals.h"
#include "StatSerializer.h"
#include "../Statistics.h"
#include "NamespaceSerializer.h"

#include <json/json.h>





// Upgrade mapping from pre-1.13 names. TODO: remove on 2020-09-18
static const std::unordered_map<std::string_view, Statistic> LegacyMapping
{
	{ "achievement.openInventory", Statistic::AchOpenInventory },
	{ "achievement.mineWood", Statistic::AchMineWood },
	{ "achievement.buildWorkBench", Statistic::AchBuildWorkBench },
	{ "achievement.buildPickaxe", Statistic::AchBuildPickaxe },
	{ "achievement.buildFurnace", Statistic::AchBuildFurnace },
	{ "achievement.acquireIron", Statistic::AchAcquireIron },
	{ "achievement.buildHoe", Statistic::AchBuildHoe },
	{ "achievement.makeBread", Statistic::AchMakeBread },
	{ "achievement.bakeCake", Statistic::AchBakeCake },
	{ "achievement.buildBetterPickaxe", Statistic::AchBuildBetterPickaxe },
	{ "achievement.cookFish", Statistic::AchCookFish },
	{ "achievement.onARail", Statistic::AchOnARail },
	{ "achievement.buildSword", Statistic::AchBuildSword },
	{ "achievement.killEnemy", Statistic::AchKillEnemy },
	{ "achievement.killCow", Statistic::AchKillCow },
	{ "achievement.flyPig", Statistic::AchFlyPig },
	{ "achievement.snipeSkeleton", Statistic::AchSnipeSkeleton },
	{ "achievement.diamonds", Statistic::AchDiamonds },
	{ "achievement.portal", Statistic::AchPortal },
	{ "achievement.ghast", Statistic::AchGhast },
	{ "achievement.blazeRod", Statistic::AchBlazeRod },
	{ "achievement.potion", Statistic::AchPotion },
	{ "achievement.theEnd", Statistic::AchTheEnd },
	{ "achievement.theEnd2", Statistic::AchTheEnd2 },
	{ "achievement.enchantments", Statistic::AchEnchantments },
	{ "achievement.overkill", Statistic::AchOverkill },
	{ "achievement.bookcase", Statistic::AchBookcase },
	{ "achievement.exploreAllBiomes", Statistic::AchExploreAllBiomes },
	{ "achievement.spawnWither", Statistic::AchSpawnWither },
	{ "achievement.killWither", Statistic::AchKillWither },
	{ "achievement.fullBeacon", Statistic::AchFullBeacon },
	{ "achievement.breedCow", Statistic::AchBreedCow },
	{ "achievement.diamondsToYou", Statistic::AchDiamondsToYou },
	{ "stat.animalsBred", Statistic::AnimalsBred },
	{ "stat.boatOneCm", Statistic::BoatOneCm },
	{ "stat.climbOneCm", Statistic::ClimbOneCm },
	{ "stat.crouchOneCm", Statistic::CrouchOneCm },
	{ "stat.damageDealt", Statistic::DamageDealt },
	{ "stat.damageTaken", Statistic::DamageTaken },
	{ "stat.deaths", Statistic::Deaths },
	{ "stat.drop", Statistic::Drop },
	{ "stat.fallOneCm", Statistic::FallOneCm },
	{ "stat.fishCaught", Statistic::FishCaught },
	{ "stat.flyOneCm", Statistic::FlyOneCm },
	{ "stat.horseOneCm", Statistic::HorseOneCm },
	{ "stat.jump", Statistic::Jump },
	{ "stat.leaveGame", Statistic::LeaveGame },
	{ "stat.minecartOneCm", Statistic::MinecartOneCm },
	{ "stat.mobKills", Statistic::MobKills },
	{ "stat.pigOneCm", Statistic::PigOneCm },
	{ "stat.playerKills", Statistic::PlayerKills },
	{ "stat.playOneMinute", Statistic::PlayOneMinute },
	{ "stat.sprintOneCm", Statistic::SprintOneCm },
	{ "stat.swimOneCm", Statistic::SwimOneCm },
	{ "stat.talkedToVillager", Statistic::TalkedToVillager },
	{ "stat.timeSinceDeath", Statistic::TimeSinceDeath },
	{ "stat.tradedWithVillager", Statistic::TradedWithVillager },
	{ "stat.walkOneCm", Statistic::WalkOneCm },
	{ "stat.diveOneCm", Statistic::WalkUnderWaterOneCm },
	{ "stat.armorCleaned", Statistic::CleanArmor },
	{ "stat.bannerCleaned", Statistic::CleanBanner },
	{ "stat.cakeSlicesEaten", Statistic::EatCakeSlice },
	{ "stat.itemEnchanted", Statistic::EnchantItem },
	{ "stat.cauldronFilled", Statistic::FillCauldron },
	{ "stat.dispenserInspected", Statistic::InspectDispenser },
	{ "stat.dropperInspected", Statistic::InspectDropper },
	{ "stat.hopperInspected", Statistic::InspectHopper },
	{ "stat.beaconInteraction", Statistic::InteractWithBeacon },
	{ "stat.brewingstandInteraction", Statistic::InteractWithBrewingstand },
	{ "stat.craftingTableInteraction", Statistic::InteractWithCraftingTable },
	{ "stat.furnaceInteraction", Statistic::InteractWithFurnace },
	{ "stat.chestOpened", Statistic::OpenChest },
	{ "stat.enderchestOpened", Statistic::OpenEnderchest },
	{ "stat.noteblockPlayed", Statistic::PlayNoteblock },
	{ "stat.recordPlayed", Statistic::PlayRecord },
	{ "stat.flowerPotted", Statistic::PotFlower },
	{ "stat.trappedChestTriggered", Statistic::TriggerTrappedChest },
	{ "stat.noteblockTuned", Statistic::TuneNoteblock },
	{ "stat.cauldronUsed", Statistic::UseCauldron },
	{ "stat.aviateOneCm", Statistic::AviateOneCm },
	{ "stat.sleepInBed", Statistic::SleepInBed },
	{ "stat.sneakTime", Statistic::SneakTime }
};





cStatSerializer::cStatSerializer(cStatManager & Manager, const std::string & WorldPath, std::string FileName) :
	m_Manager(Manager),
	m_Path(WorldPath + cFile::GetPathSeparator() + "stats")
{
	// Even though stats are shared between worlds, they are (usually) saved
	// inside the folder of the default world.

	// Ensure that the directory exists.
	cFile::CreateFolder(m_Path);

	m_Path += cFile::GetPathSeparator() + std::move(FileName) + ".json";
}





void cStatSerializer::Load(void)
{
	Json::Value Root;
	InputFileStream(m_Path) >> Root;

	LoadLegacyFromJSON(Root);
	LoadCustomStatFromJSON(Root["stats"]["custom"]);
}





void cStatSerializer::Save(void)
{
	Json::Value Root;

	SaveStatToJSON(Root["stats"]);
	Root["DataVersion"] = NamespaceSerializer::DataVersion();

	OutputFileStream(m_Path) << Root;
}





void cStatSerializer::SaveStatToJSON(Json::Value & a_Out)
{
	m_Manager.ForEachStatisticType([&a_Out](const cStatManager::CustomStore & Store)
	{
		if (Store.empty())
		{
			// Avoid saving "custom": null to disk:
			return;
		}

		auto & Custom = a_Out["custom"];
		for (const auto & Item : Store)
		{
			Custom[NamespaceSerializer::From(Item.first)] = Item.second;
		}
	});
}





void cStatSerializer::LoadLegacyFromJSON(const Json::Value & In)
{
	for (auto Entry = In.begin(); Entry != In.end(); ++Entry)
	{
		const auto & Key = Entry.key().asString();
		const auto FindResult = LegacyMapping.find(Key);

		if ((FindResult != LegacyMapping.end()) && Entry->isInt())
		{
			m_Manager.SetValue(FindResult->second, Entry->asInt());
		}
	}
}





void cStatSerializer::LoadCustomStatFromJSON(const Json::Value & a_In)
{
	for (auto it = a_In.begin() ; it != a_In.end() ; ++it)
	{
		const auto & Key = it.key().asString();
		const auto StatInfo = NamespaceSerializer::SplitNamespacedID(Key);
		if (StatInfo.first == NamespaceSerializer::Namespace::Unknown)
		{
			// Ignore non-Vanilla, non-Cuberite namespaces for now:
			continue;
		}

		const auto & StatName = StatInfo.second;
		try
		{
			m_Manager.SetValue(NamespaceSerializer::ToCustomStatistic(StatName), it->asInt());
		}
		catch (const std::out_of_range & Oops)
		{
			FLOGWARNING("Invalid statistic type \"{}\"", StatName);
		}
		catch (const Json::LogicError & Oops)
		{
			FLOGWARNING("Invalid statistic value for type \"{}\"", StatName);
		}
	}
}
