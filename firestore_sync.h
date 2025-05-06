
#ifndef FIRESTORE_SYNC_H
#define FIRESTORE_SYNC_H
#include <FirebaseClient.h>


extern FirebaseApp app;

void initFirebase();
void uploadStatusToFirestore();
void uploadRefillWaterStatusToFirestore();
void firestoreUploadCallback(AsyncResult &result);
void processData(AsyncResult &aResult);

#endif
