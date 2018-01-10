// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_inventory_dialog
    #undef _inc_inventory_dialog
#endif
// custom include-guard to ensure we don't duplicate
#if defined _inventory_dialog_included
	#endinput
#endif
#define _inventory_dialog_included


#include <a_samp>
#include <item>
#include <inventory>
#include <easyDialog>
#include <YSI\y_hooks>


// Functions


forward DisplayPlayerInventory(playerid);
/*
# Description:
Displays the inventory dialog.
*/

forward ClosePlayerInventory(playerid, call = false);
/*
# Description:
Closes the inventory dialog, optionally calls `OnPlayerCloseInventory`.
*/

forward GetPlayerSelectedInventorySlot(playerid);
/*
# Description:
Returns the selected inventory slot.
*/

forward AddInventoryListItem(playerid, itemname[]);
/*
# Description:
Adds a list item to the inventory dialog for additional actions.
*/

forward AddInventoryOption(playerid, option[]);
/*
# Description:
Adds an inventory option to an item option menu.
*/

forward GetInventoryListItems(playerid, list[], len = sizeof list);
/*
# Description:
Returns the available inventory list items.
*/

forward GetInventoryOptions(playerid, list[], len = sizeof list);
/*
# Description:
Returns the available inventory options.
*/

forward GetInventoryListItemCount(playerid);
/*
# Description:
Returns the amount of additional inventory list items.
*/

forward GetInventoryOptionCount(playerid);
/*
# Description:
Returns the amount of inventory item options.
*/

forward bool:IsPlayerViewingInventory(playerid);
/*
# Description:
Returns true if the player is in the inventory screen.
*/


// Events


forward OnPlayerOpenInventory(playerid);
/*
# Called:
When the player opens their inventory.
*/

forward OnPlayerCloseInventory(playerid);
/*
# Called:
When a player closes their inventory via close button or if `call` is set on `ClosePlayerInventory`.
*/

forward OnPlayerSelectExtraItem(playerid, item);
/*
# Called:
When a player selects an inventory list item beyond the list of items, an additional option.
*/

forward OnPlayerViewInventoryOpt(playerid);
/*
# Called:
When the player views the options for an item.
*/

forward OnPlayerSelectInventoryOpt(playerid, option);
/*
# Called:
When the player selects an option for an item.
*/


static
	inv_ItemListTotal[MAX_PLAYERS],
	inv_SelectedSlot[MAX_PLAYERS],
	inv_ViewingInventory[MAX_PLAYERS],
	inv_ExtraItemList[MAX_PLAYERS][128],
	inv_ExtraItemCount[MAX_PLAYERS],
	inv_OptionsList[MAX_PLAYERS][128],
	inv_OptionsCount[MAX_PLAYERS];


stock DisplayPlayerInventory(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return 1;
	}

	new
		title[18],
		list[(MAX_INVENTORY_SLOTS * (MAX_ITEM_NAME + MAX_ITEM_TEXT + 1)) + 32],
		itemid,
		tmp[5 + MAX_ITEM_NAME + MAX_ITEM_TEXT + 1],
		size;

	inv_ItemListTotal[playerid] = 0;

	new ret = GetPlayerInventorySize(playerid, size);
	if(ret != 0) {
		return 2;
	}

	for(new i; i < size; i++) {
		ret = GetInventorySlotItem(playerid, i, itemid);

		if(!IsValidItem(itemid) || ret != 0) {
			break;
		}

		GetItemName(itemid, tmp);

		format(list, sizeof(list), "%s[%02d]%s\n", list, GetItemTypeSize(GetItemType(itemid)), tmp);
		inv_ItemListTotal[playerid]++;
	}

	new slots;
	ret = GetInventoryFreeSlots(playerid, slots);

	for(new i; i < slots; i++) {
		strcat(list, "<Empty>\n");
		inv_ItemListTotal[playerid]++;
	}

	inv_ExtraItemList[playerid][0] = EOS;
	inv_ExtraItemCount[playerid] = 0;

	if(CallLocalFunction("OnPlayerOpenInventory", "d", playerid)) {
		return 3;
	}

	if(!isnull(inv_ExtraItemList[playerid])) {
		strcat(list, inv_ExtraItemList[playerid]);
	}

	format(title, sizeof(title), "Inventory (%d/%d)", size - slots, size);

	Dialog_Show(playerid, _inv_inventoryList, DIALOG_STYLE_LIST, title, list, "Options", "Close");

	inv_ViewingInventory[playerid] = true;

	return 0;
}

Dialog:_inv_inventoryList(playerid, response, listitem, inputtext[]) {
	if(!response) {
		ClosePlayerInventory(playerid, true);
	}

	if(listitem >= inv_ItemListTotal[playerid]) {
		CallLocalFunction("OnPlayerSelectExtraItem", "dd", playerid, listitem - inv_ItemListTotal[playerid]);
		inv_ViewingInventory[playerid] = false;
		return 1;
	}

	new
		itemid,
		ret;
	
	ret = GetInventorySlotItem(playerid, listitem, itemid);

	if(!IsValidItem(itemid) || ret != 0) {
		DisplayPlayerInventory(playerid);
	} else {
		inv_SelectedSlot[playerid] = listitem;
		_inv_displaySlotOptions(playerid, listitem);
	}

	return 1;
}

stock ClosePlayerInventory(playerid, call = false) {
	if(!inv_ViewingInventory[playerid]) {
		return 1;
	}

	if(call) {
		if(CallLocalFunction("OnPlayerCloseInventory", "d", playerid)) {
			DisplayPlayerInventory(playerid);
			return 2;
		}
	}

	ShowPlayerDialog(playerid, -1, 0, NULL, NULL, NULL, NULL);
	inv_ViewingInventory[playerid] = false;

	return 0;
}

stock GetPlayerSelectedInventorySlot(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return -1;
	}

	return inv_SelectedSlot[playerid];
}

stock AddInventoryListItem(playerid, itemname[]) {
	if(strlen(inv_ExtraItemList[playerid]) + strlen(itemname) > sizeof(inv_ExtraItemList[])) {
		return 0;
	}

	strcat(inv_ExtraItemList[playerid], itemname);
	strcat(inv_ExtraItemList[playerid], "\n");

	return inv_ExtraItemCount[playerid]++;
}

stock AddInventoryOption(playerid, option[]) {
	if(strlen(inv_OptionsList[playerid]) + strlen(option) > sizeof(inv_OptionsList[])) {
		return 0;
	}

	strcat(inv_OptionsList[playerid], option);
	strcat(inv_OptionsList[playerid], "\n");

	return inv_OptionsCount[playerid]++;
}

stock GetInventoryListItems(playerid, list[], len = sizeof list) {
	if(!IsPlayerConnected(playerid)) {
		return 1;
	}

	memcpy(list, inv_ExtraItemList[playerid], 0, 4 * inv_ExtraItemCount[playerid], len);

	return 0;
}

stock GetInventoryOptions(playerid, list[], len = sizeof list) {
	if(!IsPlayerConnected(playerid)) {
		return 1;
	}

	memcpy(list, inv_OptionsList[playerid], 0, 4 * inv_OptionsCount[playerid], len);

	return 0;
}

stock GetInventoryListItemCount(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return 0;
	}

	return inv_ExtraItemCount[playerid];
}

stock GetInventoryOptionCount(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return 0;
	}

	return inv_OptionsCount[playerid];
}

stock bool:IsPlayerViewingInventory(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return 0;
	}

	return inv_ViewingInventory[playerid];
}


/*==============================================================================

	Internal Functions and Hooks

==============================================================================*/


hook OnScriptInit() {
	for(new i; i < MAX_PLAYERS; i++) {
		for(new j; j < MAX_INVENTORY_SLOTS; j++) {
			inv_SelectedSlot[i] = -1;
		}
	}
}

hook OnPlayerConnect(playerid) {
	for(new j; j < MAX_INVENTORY_SLOTS; j++) {
		inv_SelectedSlot[playerid] = -1;
	}
}

_inv_displaySlotOptions(playerid, slotid) {
	new
		name[MAX_ITEM_NAME + MAX_ITEM_TEXT],
		itemid,
		ret;
	
	ret = GetInventorySlotItem(playerid, slotid, itemid);
	if(ret != 0) {
		return 1;
	}

	GetItemName(itemid, name);
	inv_OptionsList[playerid] = "Equip\nUse\nDrop\n";
	inv_OptionsCount[playerid] = 0;

	CallLocalFunction("OnPlayerViewInventoryOpt", "d", playerid);

	Dialog_Show(playerid, _inv_optionsList, DIALOG_STYLE_LIST, name, inv_OptionsList[playerid], "Accept", "Back");

	return 0;
}

Dialog:_inv_optionsList(playerid, response, listitem, inputtext[]) {
	if(!response) {
		DisplayPlayerInventory(playerid);
		return 1;
	}

	switch(listitem) {
		case 0: {
			if(GetPlayerItem(playerid) == INVALID_ITEM_ID) {
				new
					itemid,
					ret;
				
				ret = GetInventorySlotItem(playerid, inv_SelectedSlot[playerid], itemid);
				if(ret != 0) {
					err("failed to get inventory slot item", _i("playerid", playerid), _i("slotid", inv_SelectedSlot[playerid]), _i("ret", ret));
					return 1;
				}

				RemoveItemFromInventory(playerid, inv_SelectedSlot[playerid]);
				GiveWorldItemToPlayer(playerid, itemid, 1);
				DisplayPlayerInventory(playerid);
			} else {
				ShowActionText(playerid, "You are already holding something", 3000, 200);
				DisplayPlayerInventory(playerid);
			}
		}
		case 1: {
			if(GetPlayerItem(playerid) == INVALID_ITEM_ID) {
				new
					itemid,
					ret;
				
				ret = GetInventorySlotItem(playerid, inv_SelectedSlot[playerid], itemid);
				if(ret != 0) {
					err("failed to get inventory slot item", _i("playerid", playerid), _i("slotid", inv_SelectedSlot[playerid]), _i("ret", ret));
					return 1;
				}

				RemoveItemFromInventory(playerid, inv_SelectedSlot[playerid]);
				GiveWorldItemToPlayer(playerid, itemid, 1);

				PlayerUseItem(playerid);

				ClosePlayerInventory(playerid, true);
			} else {
				ShowActionText(playerid, "You are already holding something", 3000, 200);
				DisplayPlayerInventory(playerid);
			}
		}
		case 2: {
			if(GetPlayerItem(playerid) == INVALID_ITEM_ID) {
				new
					itemid,
					ret;
				
				ret = GetInventorySlotItem(playerid, inv_SelectedSlot[playerid], itemid);
				if(ret != 0) {
					err("failed to get inventory slot item", _i("playerid", playerid), _i("slotid", inv_SelectedSlot[playerid]), _i("ret", ret));
					return 1;
				}

				RemoveItemFromInventory(playerid, inv_SelectedSlot[playerid]);
				GiveWorldItemToPlayer(playerid, itemid, 1);

				PlayerDropItem(playerid);

				ClosePlayerInventory(playerid, true);
			} else {
				ShowActionText(playerid, "You are already holding something", 3000, 200);
				DisplayPlayerInventory(playerid);
			}
		}
		default: {
			CallLocalFunction("OnPlayerSelectInventoryOpt", "dd", playerid, listitem - 3);
		}
	}

	return 1;
}