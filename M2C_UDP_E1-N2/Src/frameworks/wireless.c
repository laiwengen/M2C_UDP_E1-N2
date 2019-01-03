#include "stdint.h"
#include "stdlib.h"
#include "frameworks/wireless.h"
#include "frameworks/package.h"
#include "frameworks/number.h"
#include "frameworks/strings.h"


static package_pair_t* removeFirstPairByKey(package_element_t* e, wireless_key_t key) {
	package_element_t* keyE = wireless_new(key);
	if (keyE) {
		return package_removeFirstPairByKey(e, keyE);
	}
	return 0;
}


static package_pair_t* safeAddObject(package_element_t* e, wireless_key_t key, package_element_t* valueE);

static package_element_t* fesert(package_element_t* rootE, wireless_key_t key, uint8_t isObject) {
	if (rootE == 0) {
		return 0;
	}
	package_element_t* data = wireless_findValueByKey(rootE, key);
	if (data == 0) {
		package_pair_t* pair = safeAddObject(rootE, key, isObject?package_newObject():package_newArray());
		if (pair) {
			package_setFlag(pair, ignoreEmpty);
		}
		data = wireless_findValueByKey(rootE, key);
	}
	return data;
}

static uint8_t addToClassObject(wireless_t* wireless, wireless_key_t class, package_element_t* keyE, package_element_t* valueE) {
	if (wireless == 0 || wireless->uploadPackage == 0) {
		return 0;
	}
	package_element_t* rootE = wireless->uploadPackage;
	package_element_t* dataE = fesert(rootE, class, 1);
	if (dataE == 0) {
		return 0;
	}
	package_pair_t* existsP = package_removeFirstPairByKey(dataE,keyE);
	if (existsP) {
		package_deletePair(existsP);
	}
	package_pair_t* newPair = package_addKeyValue(dataE, keyE, valueE);
	if (!newPair) {
		return 0;
	}
	return 1;
}
static package_pair_t* safeAddObject(package_element_t* e, wireless_key_t key, package_element_t* valueE) {
	if (valueE == 0) {
		return 0;
	}
	if (e) {
		package_element_t* keyE = wireless_new(key);
		if (keyE) {
			package_pair_t* pair = package_addKeyValue(e, keyE, valueE);
			if (pair) {
				return pair;
			} else {
				package_delete(keyE);
			}
		}
	}
	package_delete(valueE);
	return 0;
}

static package_pair_t* safeAddNumber(package_element_t* e, wireless_key_t key, int32_t number, int16_t e10) {
	package_element_t* valueE;
	if (e10==0) {
		valueE = package_newInt32(number);
	} else {
		valueE = package_newDec32(number,e10);
	}
	return safeAddObject(e, key, valueE);
}
static package_pair_t* safeAddString(package_element_t* e, wireless_key_t key, char const * str) {
	package_element_t* valueE = package_newString(str);
	return safeAddObject(e, key, valueE);
}

static void setLine(wireless_t* wireless, uint8_t type, uint16_t line) {
	uint16_t* lines = (&wireless->downloadLine);
	lines[type] = line;
	if (wireless->lineChangedFunction) {
		wireless->lineChangedFunction(wireless, type, line);
	}
}

package_element_t* wireless_safeAddObject(package_element_t* e, wireless_key_t key, package_element_t* valueE) {
	return safeAddObject(e, key, valueE)?valueE:0;
}

package_pair_t* package_findPairByKey(package_element_t* parent, wireless_key_t key) {
	package_element_t* ok = wireless_new(key);
	package_pair_t* toRet = package_findFirstPairByKey(parent, ok);
	package_delete(ok);
	return toRet;
}

uint8_t wireless_keyEqual(package_element_t* a, wireless_key_t b) {
#if WIRELESS_BIN
	return package_equalNumber(a, b, 0);
#else
	return package_equalString(a, b);
#endif
}
//wireless builder
//wireless send
uint8_t wireless_setDeviceId(wireless_t* wireless, char* deviceId) {
	return safeAddString(wireless->uploadPackage, wireless_key(root,deviceId),deviceId) != 0;
}

uint8_t wireless_setVersion(wireless_t* wireless, char const* hardware, char const* firmware, char const* protocol) {
	package_element_t* version = package_newObject();
	if (!version) {
		return 0;
	}
	if (!safeAddString(version,wireless_key(version,hardware),hardware)
		|| !safeAddString(version,wireless_key(version,firmware),firmware)
		|| !safeAddString(version,wireless_key(version,protocol),protocol)) {
		package_delete(version);
		return 0;
	}
	return safeAddObject(wireless->uploadPackage, wireless_key(root, version), version)!=0;
}

uint8_t wireless_addCommand(wireless_t* wireless, package_element_t* keyE, package_element_t* valueE, uint32_t time) {
	if (wireless == 0 || wireless->uploadPackage == 0) {
		return 0;
	}
	package_element_t* rootE = wireless->uploadPackage;
	package_element_t* classE = fesert(rootE, wireless_key(root,commands), 0);
	if (classE == 0) {
		return 0;
	}
	package_element_t* commandE = package_newObject();
	if (!commandE) {
		return 0;
	}
	package_pair_t* timeP = safeAddNumber(commandE, wireless_key(command,time), time, 0);
	if (timeP) {
		package_setFlag(timeP,ignore);
	} else {
		package_delete(commandE);
		return 0;
	}
	if (!safeAddNumber(commandE, wireless_key(command,delayed), 0, 0)) {
		package_delete(commandE);
		return 0;
	}
	if (!safeAddNumber(commandE, wireless_key(command,line), wireless->uploadLineGenerator, 0)) {
		package_delete(commandE);
		return 0;
	}
	package_element_t* commandDataE = fesert(commandE, wireless_key(command,data), 1);
	if (commandDataE == 0) {
		package_delete(commandE);
		return 0;
	}
	if (!package_addKeyValue(commandDataE, keyE, valueE)) {
		package_delete(commandE);
		return 0;
	}
	if (!package_addElement(classE,commandE)) {
		package_delete(commandE);
		return 0;
	}
	setLine(wireless, WIRELESS_LINE_TYPE_uploadGenerator, wireless->uploadLineGenerator + 1);
	return 1;
}

uint8_t wireless_addRequest(wireless_t* wireless, package_element_t* key, package_element_t* value) {
	if (addToClassObject(wireless, wireless_key(root,requests), key, value)) {
		package_element_t* classE = wireless_findValueByKey(wireless->uploadPackage, wireless_key(root,requests));
		return 1;
	}
	return 0;
}

uint8_t wireless_addData(wireless_t* wireless, package_element_t* key, package_element_t* value) {
	return addToClassObject(wireless, wireless_key(root,data), key, value);
}

package_element_t* wireless_getRequest(wireless_t* wireless, wireless_key_t key) {
	package_element_t* classE = wireless_findValueByKey(wireless->uploadPackage, wireless_key(root,requests));
	if (!classE) {
		return 0;
	}
	return wireless_findValueByKey(classE, key);
}

int16_t wireless_getRequestsCount(wireless_t* wireless) {
	package_element_t* classE = wireless_findValueByKey(wireless->uploadPackage, wireless_key(root,requests));
	if (!classE) {
		return 0;
	}
	return package_getPairCount(classE);
}
int16_t wireless_getCommandsCount(wireless_t* wireless) {
	package_element_t* classE = wireless_findValueByKey(wireless->uploadPackage, wireless_key(root,commands));
	if (!classE) {
		return 0;
	}
	return package_getElementCount(classE);
}

static uint8_t identifyMatched(package_element_t* a, package_element_t* b) {
	uint8_t deviceIdMatch = package_equal(wireless_findValueByKey(a,wireless_key(root,deviceId)),
	wireless_findValueByKey(b,wireless_key(root,deviceId)));
	uint8_t sessionIdMatch = package_equal(wireless_findValueByKey(a,wireless_key(root,sessionId)),
	wireless_findValueByKey(b,wireless_key(root,sessionId)));
	if (deviceIdMatch || sessionIdMatch) {
		return 1;
	}
	return 0;
}

void wireless_received(wireless_t* wireless, uint8_t* buffer, uint16_t size) {
#if WIRELESS_BIN
	package_element_t* receivedE = package_deserialize(buffer, size);
#else
	package_element_t* receivedE = package_parseJson((char const*)buffer, 0);
	// char* str = string_duplicate((char const*)buffer, size);
	// // char* strt = "{\"deviceId\":\"M3-1e4687cc490bf9ecc0e62b879feaa2b0\",\"responses\":{\"time\":{\"data\":{\"timestamp\":1519544754,\"year\":2018,\"month\":2,\"day\":25,\"hour\":15,\"minute\":45,\"second\":54,\"weekday\":7}}}}";
	// if (!str) {
	// 	return;
	// }
	// package_element_t* receivedE = package_parseJson(str, 0);
	// free(str);
#endif
#if 1
	if (!receivedE || receivedE->type != PACKAGE_TYPE_object) {
		package_delete(receivedE);
		return;
	}
	package_element_t* contentsE = wireless_findValueByKey(receivedE, wireless_key(contents,contents));
	if (!contentsE || contentsE->type != PACKAGE_TYPE_array) {
		package_delete(receivedE);
		return;
	}
	fortl (package_element_t*, package_elementPointer(contentsE, list_t*, 0)) {
		if (identifyMatched(v, wireless->uploadPackage)) {
			package_element_t* rootE = v;
			// route TODO done
			{
				package_element_t* routeE = wireless_findValueByKey(rootE, wireless_key(root,route));
				if (routeE) {
					package_element_t* addressE = wireless_findValueByKey(routeE, wireless_key(route,address));
					package_element_t* portE = wireless_findValueByKey(routeE, wireless_key(route,port));
					if (addressE && addressE->type == PACKAGE_TYPE_string && portE && portE->type == PACKAGE_TYPE_integer) {
						wireless->routeFunction(wireless, package_elementPointer(addressE, char, 0), package_getInt(portE));
					}
				}
			}
			// setSession TODO done
			{
				package_element_t* setSessionIdE = wireless_findValueByKey(rootE, wireless_key(root,setSessionId));
				if (setSessionIdE) {
					package_element_t* sessionIdE = 0;
					if (setSessionIdE->type == PACKAGE_TYPE_bytes) {
						sessionIdE = package_newBytes(package_elementPointer(setSessionIdE,uint8_t,0), setSessionIdE->size);
					}
					if (setSessionIdE->type == PACKAGE_TYPE_string) {
						sessionIdE = package_newBytes(package_elementPointer(setSessionIdE,uint8_t,0), setSessionIdE->size - 1);
					}
					if (sessionIdE) {
						if (safeAddObject(wireless->uploadPackage, wireless_key(root,sessionId), sessionIdE)){
							package_deletePair(removeFirstPairByKey(wireless->uploadPackage, wireless_key(root,deviceId)));
							package_deletePair(removeFirstPairByKey(wireless->uploadPackage, wireless_key(root,version)));
							package_deletePair(removeFirstPairByKey(wireless->uploadPackage, wireless_key(root,routers)));
						}
					}
				}
			}
			// need Identify TODO done
			{
				package_element_t* errorE = wireless_findValueByKey(rootE, wireless_key(root,error));
				if (errorE) {
					package_element_t* codeE = wireless_findValueByKey(errorE, wireless_key(error,code));
					if (codeE && package_getInt(codeE) == WIRELESS_ERROR_code_sessionIdTimeout) {
						package_deletePair(removeFirstPairByKey(wireless->uploadPackage, wireless_key(root,sessionId)));
						wireless->initFunction(wireless);
					}
				}
			}

			//request & response
			package_element_t* key, * value;
			if (wireless->uploadPackage && wireless->responseFunction) {
				package_element_t* responses = wireless_findValueByKey(rootE, wireless_key(root,responses));
				package_element_t* requests = wireless_findValueByKey(wireless->uploadPackage, wireless_key(root,requests));
				if (responses && requests) {
					list_t* iter = package_getFirstPair(responses, &key, &value);
					while (iter) {
						// deal with key values
						if (key && value) {
							package_pair_t* pair = package_removeFirstPairByKey(requests, key);
							if (pair) {
								wireless->responseFunction(wireless, key, pair->value, value);
								package_deletePair(pair);
							}
						}
						iter = package_getNextPair(iter, &key, &value);
					}
				}
			}
			// download commands
			{
				package_element_t* commandsE = wireless_findValueByKey(rootE, wireless_key(root,commands));;
				if (commandsE) {
					list_t* iter = package_getFirstElement(commandsE, &value);
					uint16_t downloadLine = wireless->downloadLine;
					while (iter) {
						package_element_t* v = value;
						iter = package_getNextElement(iter, &value);
						if (v) {
							package_element_t* lineE = wireless_findValueByKey(v, wireless_key(command,line));
							if (lineE != 0) {
								if (lineE->type == PACKAGE_TYPE_integer) {
									uint16_t line = package_getInt(lineE);
									if (number_circleCompare(line, downloadLine, UINT16_MAX) >= 0) {
										downloadLine = line + 1;
										if (wireless->commandFunction) {
											uint32_t delayed = UINT32_MAX;
											uint8_t by = UINT8_MAX;
											package_element_t* delayedE = wireless_findValueByKey(v, wireless_key(command,delayed));
											if (delayedE != 0) {
												if (delayedE->type == PACKAGE_TYPE_integer) {
													delayed = package_getInt(delayedE);
												}
											}
											package_element_t* byE = wireless_findValueByKey(v, wireless_key(command,by));
											if (byE != 0) {
												if (byE->type == PACKAGE_TYPE_integer) {
													by = package_getInt(byE);
												}
											}
											wireless->commandFunction(wireless, wireless_findValueByKey(v, wireless_key(command,data)), delayed, by);
										}
									}
								}
							}
						}
					}
					setLine(wireless, WIRELESS_LINE_TYPE_download, downloadLine);
				}

			}
			// uploadPackage commands
			if (wireless->uploadPackage) {
				package_element_t* lineE = wireless_findValueByKey(rootE, wireless_key(root,line));
				if (lineE && lineE->type == PACKAGE_TYPE_integer) {
					uint16_t line = package_getInt(lineE);
					package_element_t* commands = wireless_findValueByKey(wireless->uploadPackage, wireless_key(root,commands));
					if (commands) {
						list_t* iter = package_getFirstElement(commands, &value);
						while (iter) {
							package_element_t* v = value;
							iter = package_getNextElement(iter, &value);
							if (v) {
								package_element_t* le = wireless_findValueByKey(v, wireless_key(command,line));
								if (le != 0 && le->type == PACKAGE_TYPE_integer) {
									uint16_t l = package_getInt(le);
									if (number_circleCompare(line, l, UINT16_MAX) > 0) {
										uint32_t delayed = UINT32_MAX;
										uint8_t by = UINT8_MAX;
										package_element_t* delayedE = wireless_findValueByKey(value, wireless_key(command,delayed));
										if (delayedE != 0) {
											if (delayedE->type == PACKAGE_TYPE_integer) {
												delayed = package_getInt(delayedE);
											}
										}
										package_element_t* byE = wireless_findValueByKey(value, wireless_key(command,by));
										if (byE != 0) {
											if (byE->type == PACKAGE_TYPE_integer) {
												by = package_getInt(byE);
											}
										}
										wireless->acceptedFunction(wireless, wireless_findValueByKey(v, wireless_key(command,data)), delayed, by);
										package_element_t* re = package_removeFirstElementByPointer(commands, v);
										package_delete(re);
									}
								}
							}
						}
					}
					// command
					if (number_circleCompare(line, wireless->uploadLineGenerator, UINT16_MAX) > 0) {
						setLine(wireless, WIRELESS_LINE_TYPE_uploadGenerator, line);
					}
				}
			}
		} else {
			package_element_t* subPackage = 0;
			package_element_t* download = v;
			fortl(package_element_t*,&wireless->subPackages) {
				if (identifyMatched(download, v)) {
					subPackage = v;
					break;
				}
			}
			if (subPackage) {
				if(wireless->subPackageFunction) {
					if (wireless->subPackageFunction(wireless, subPackage, download)) {
						list_remove(package_elementPointer(contentsE, list_t*, 0), v);
					}
				}
			}
			// TODO other download
		}
	}
	#endif
	package_delete(receivedE);
}
void generateDalyed(package_element_t* e, uint32_t time) {
	if (e ==0 || e->type != PACKAGE_TYPE_array || time == 0) {
		return;
	}
	fortl(package_element_t*, package_elementPointer(e, list_t*, 0)) {
		package_element_t* timeE = wireless_findValueByKey(v, wireless_key(command,time));
		package_element_t* delayedE = wireless_findValueByKey(v, wireless_key(command,delayed));
		if (timeE && delayedE) {
			uint32_t t = package_getInt(timeE);
			package_pair_t* delayedP= package_findPairByKey(v, wireless_key(command,delayed));
			uint32_t delayed;
			if (t && (time - t) < INT32_MAX / 1000) {
				delayed = (time - t) * 1000;
				package_clearFlag(delayedP,ignore);
			} else {
				delayed = UINT32_MAX;
				package_setFlag(delayedP,ignore);
			}
			package_setNumber(delayedE, delayed, 0);
		}
	}
}

int16_t wireless_fetchBytes(wireless_t* wireless, uint32_t time, uint8_t** out) {
	package_element_t* rootE = package_newObject();
	package_element_t* contentsKE = wireless_new(wireless_key(contents,contents));
	package_element_t* contentsVE = package_newArray();

	if (!rootE || !contentsKE || !contentsVE || !package_addKeyValue(rootE, contentsKE, contentsVE)) {
		package_delete(contentsKE);
		package_delete(contentsVE);
		package_delete(rootE);
		return 0;
	}
	if (!package_addElement(contentsVE, wireless->uploadPackage)) {
		package_delete(rootE);
		return 0;
	}
	package_element_t* lineE = wireless_findValueByKey(wireless->uploadPackage,wireless_key(root,line));
	if (lineE) {
		package_setNumber(lineE, wireless->downloadLine, 0);
	}
	generateDalyed(wireless_findValueByKey(wireless->uploadPackage,wireless_key(root, commands)),time);
	#if WIRELESS_BIN
		int16_t size = package_serialize(rootE, out);
	#else
		int16_t size = package_toJson(rootE, (char**)out);
	#endif
	package_removeFirstElementByPointer(contentsVE, wireless->uploadPackage);
	package_delete(rootE);
	// int16_t size = package_toJson(wireless->uploadPackage, (char**)out);
	package_element_t* ke = wireless_new(wireless_key(root,data));
	if (ke) {
		package_pair_t* dataPair = package_removeFirstPairByKey(wireless->uploadPackage, ke);
		if (dataPair) {
			package_deletePair(dataPair);
		}
		package_delete(ke);
	}
	return size;
}
//wireless parse

// void wireless_setup(wireless_t* wireless, )

void wireless_init(wireless_t* wireless) {
	wireless->uploadPackage = package_newObject();
	wireless->initFunction(wireless);
	safeAddObject(wireless->uploadPackage, wireless_key(root,line), package_newInt32(wireless->downloadLine));
}
