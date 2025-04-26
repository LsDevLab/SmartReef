
#ifndef FIRESTORE_SYNC_H
#define FIRESTORE_SYNC_H
#include <FirebaseClient.h>

#define FIREBASE_HOST "hi-chat-bec8f-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAA87TyI0UwyTGlcfB4QHnp9OaLcHVL9js"
#define FIREBASE_PROJECT_ID "hi-chat-bec8f"
#define USER_EMAIL ""
#define USER_PASSWORD ""
#define DATABASE_SECRET "Q1Tok1ZEJmHfmk9YHzhiowC3ly2szlZQBk8GvDTx"

extern FirebaseApp app;

void initFirebase();
void uploadStatusToFirestore();
void uploadRefillWaterStatusToFirestore();

#endif
