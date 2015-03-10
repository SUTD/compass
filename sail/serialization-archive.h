/*
 * serialization-archive.h
 *
 *  Created on: Jan 16, 2012
 *      Author: tdillig
 */

#ifndef SERIALIZATION_ARCHIVE_H_
#define SERIALIZATION_ARCHIVE_H_

/*
 * Archive type used for boost serialization
 */
//#define IN_ARCHIEVE boost::archive::text_iarchive
//#define OUT_ARCHIEVE boost::archive::text_oarchive

#define IN_ARCHIEVE boost::archive::binary_iarchive
#define OUT_ARCHIEVE boost::archive::binary_oarchive


#endif /* SERIALIZATION_ARCHIVE_H_ */
