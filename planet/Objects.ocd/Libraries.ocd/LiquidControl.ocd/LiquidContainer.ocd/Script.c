/**
 * Liquid Container
 *
 * Basic interface for anything that can contain liquids.
 *
 * Author: Ringwaul, ST-DDT, Marky
 */

local liquid_container_item;

// -------------- Properties
//
// Simple properties that define the object as a liquid container,
// what kind of liquid it can hold, how much it can hold
//
// naming scheme: [verb]LiquidContainer[attribute], because it concerns the container

func IsLiquidContainer() { return true;}

func IsLiquidContainerForMaterial(string liquid_name)
{
	return false;
}

func GetLiquidContainerMaxFillLevel()
{
	return 0;
}

// -------------- Current Status
//
// Simple boolean status checks
//
// naming scheme: LiquidContainer[attribute/question]

func LiquidContainerIsEmpty()
{
	return (GetLiquidFillLevel() == 0);
}

func LiquidContainerIsFull()
{
	return GetLiquidFillLevel() == GetLiquidContainerMaxFillLevel();
}

func LiquidContainerAccepts(string liquid_name)
{
	return this->IsLiquidContainerForMaterial(liquid_name)
	   && (LiquidContainerIsEmpty() || GetLiquidType() == liquid_name);
}

// -------------- Getters
//
// Getters for stored liquid and amount
// - these should be used primarily by objects that include this library
//
// naming scheme: GetLiquid[attribute], because it concerns the liquid

func GetLiquidItem()
{
	return liquid_container_item;
}


func TransferLiquidItem(object source)
{
	if (source && source.IsLiquid != nil)
	{
		var liquid = source->IsLiquid();
		
		if (liquid && !LiquidContainerAccepts(liquid)) return false;
		
		var remaining = GetLiquidFillLevelRemaining();
		if (source->GetLiquidAmount() <= remaining)
		{
			if (!GetLiquidItem())
			{
				SetLiquidItem(source);
			}
			else
			{
				var extracted = source->RemoveLiquid(nil, nil, this);
				PutLiquid(extracted[0], extracted[1]);
			}
			return true;
		}
		else
		{
			var extracted = source->RemoveLiquid(nil, remaining, this);
			if (!GetLiquidItem()) SetLiquidType(extracted[0]); // create liquid item if necessary
			PutLiquid(extracted[0], extracted[1]);
			return false;
		}
	}
	return false;
}


func SetLiquidItem(object item)
{
	if (item && (item->~IsLiquid() || item.IsLiquid != nil))
	{
		liquid_container_item = item;
	}
	else
	{
		FatalError(Format("Object %v is not a liquid", item));
	}
}

func ResetLiquidItem()
{
	liquid_container_item = nil;
	this->~UpdateLiquidContainer();
}

func GetLiquidType()
{
	if (GetLiquidItem())
	{
		return GetLiquidItem()->IsLiquid();
	}
	return nil;
}

func GetLiquidFillLevel()
{
	if (GetLiquidItem())
	{
		return GetLiquidItem()->GetLiquidAmount();
	}
	return 0;
}

func GetLiquidFillLevelRemaining()
{
	return GetLiquidContainerMaxFillLevel() - GetLiquidFillLevel();
}

// -------------- Setters
//
// Setters for stored liquid and amount
// - these should be used primarily by objects that include this library
//
// naming scheme: SetLiquid[attribute], because it concerns the liquid

func SetLiquidType(string liquid_name)
{
	var amount = 0; // for new items only
	if (GetLiquidItem())
	{
		amount = GetLiquidItem()->GetLiquidAmount();
		if (!WildcardMatch(liquid_name, GetLiquidItem()->IsLiquid()))
			GetLiquidItem()->RemoveObject();
	}

	if (!GetLiquidItem())
	{
		var item = Library_Liquid->CreateLiquid(liquid_name, amount);
		if (amount > 0) item->UpdateLiquidObject();
		// if not removed because of amount
		if (item) item->Enter(this);
	}
}

func SetLiquidFillLevel(int amount)
{
	if (!GetLiquidItem())
	{
		SetLiquidType(nil);
	}

	ChangeLiquidFillLevel(amount - GetLiquidFillLevel());
}

func ChangeLiquidFillLevel(int amount)
{
	if (GetLiquidItem())
	{
		GetLiquidItem()->DoLiquidAmount(amount);
	}
	
	this->~UpdateLiquidContainer();
}

// -------------- Interaction
//
// Interfaces for interaction with other objects

/**
Extracts liquid from the container.
@param liquid_name: Material to extract; Wildcardsupport
               Defaults to the current liquid if 'nil' is passed.
@param amount: Max Amount of liquid being extracted;
               Defaults to all contained liquid if 'nil' is passed.
@param destination: Object that extracts the liquid
@return [returned_liquid, returned_amount]
	   - returned_liquid: Material being extracted
	   - returned_amount: Amount being extracted
*/
func RemoveLiquid(string liquid_name, int amount, object destination)
{
	if (amount < 0)
	{
		FatalError(Format("You can remove positive amounts of liquid only, got %d", amount));
	}

	if (GetLiquidItem())
	{
		return GetLiquidItem()->RemoveLiquid(liquid_name, amount, destination);
	}
	else
	{
		return [nil, 0];
	}
}

/** 
Inserts liquid into the container.
@param liquid_name: Material to insert
@param amount: Max Amount of Material being inserted 
@param source: Object which inserts the liquid
@return returned_amount: The inserted amount
*/
func PutLiquid(string liquid_name, int amount, object source)
{
	if (amount < 0)
	{
		FatalError(Format("You can insert positive amounts of liquid only, got %d", amount));
	}

	TransferLiquidItem(source);
	if (!GetLiquidItem() && LiquidContainerAccepts(liquid_name))
	{
		SetLiquidType(liquid_name);
	}

	if (GetLiquidItem())
	{
		return GetLiquidItem()->PutLiquid(liquid_name, amount, source);
	}
	else //does not have a liquid item yet?
	{
		return 0;
	}
}

// --------------  Internals --------------
//
// Internal stuff


func SaveScenarioObject(props)
{
	if (!inherited(props, ...)) return false;
	if (GetLiquidType())
		props->AddCall("Fill", this, "SetLiquidContainer", Format("%v", GetLiquidType()), GetLiquidFillLevel());
	return true;
}

// set the current state, without sanity checks
func SetLiquidContainer(string liquid_name, int amount)
{
	SetLiquidType(liquid_name);
	SetLiquidFillLevel(amount);
}

// lose the liquid item if it exits the container
func Ejection(object item)
{
	if (item == GetLiquidItem())
		ResetLiquidItem();
	_inherited(...);
}