/**
		Copyright (C) 2012 Nils Weiss, Patrick Bruenn.

    This file is part of Wifly_Light.

    Wifly_Light is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wifly_Light is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wifly_Light.  If not, see <http://www.gnu.org/licenses/>. */

#include "BroadcastReceiver.h"
#include <unistd.h>
#include <jni.h>

extern "C" {
jlong Java_biz_bruenn_WiflyLight_WiflyLightActivity_getNumRemotes(JNIEnv* env, jobject ref, jlong pNative)
{
	sleep(5);
	return ((BroadcastReceiver*)pNative)->NumRemotes();
}

jlong Java_biz_bruenn_WiflyLight_WiflyLightActivity_createBroadcastReceiver(JNIEnv* env, jobject ref)
{
	return (jlong) new BroadcastReceiver(BroadcastReceiver::BROADCAST_PORT);
}

void Java_biz_bruenn_WiflyLight_WiflyLightActivity_releaseBroadcastReceiver(JNIEnv* env, jobject ref, jlong pNative)
{
	delete (BroadcastReceiver*)pNative;
}
}

