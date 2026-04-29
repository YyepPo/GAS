// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/AttributeInformation.h"

FAuraAttributeInfo UAttributeInformation::FindAttributeInfoForTag(const FGameplayTag& AttributeTag) const
{
	for (const FAuraAttributeInfo& Info : AttributeInformation)
	{
		if (Info.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return Info;
		}
	}

	return FAuraAttributeInfo();
}
