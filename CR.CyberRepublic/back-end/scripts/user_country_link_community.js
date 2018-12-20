
// MAKE SURE YOU RUN BUILD BEFORE THIS

// this should be run from the parent back-end folder, not scripts

// this is what sets the process.env
require('../dist/src/config');

const constant = require('../dist/src/constant').constant
const _ = require('lodash')
const geo = require('../dist/src/utility/geo').geo

/**
 * Script to ensure all users have a community linked
 *
 * 1. Create country communities that don't exist yet
 * 2.
 */
;(async () => {

    const db = await require('../dist/src/db').default
    const DB = await db.create()

    const db_user = DB.getModel('User')
    const db_community = DB.getModel('Community')
    const db_user_community = DB.getModel('User_Community')

    try {
        // get all user countries
        let allCountryGeos = await db_user.db.collection.distinct('profile.country')

        // get existing countries
        let existingCountries = await db_community.db.collection.distinct('geolocation', {type: constant.COMMUNITY_TYPE.COUNTRY})

        // create the missing countries
        for (let missingCountry of _.difference(allCountryGeos, existingCountries)) {

            await db_community.save({
                leaderIds: [],
                parentCommunityId: null,
                geolocation: missingCountry,
                name: geo.geolocationMap[missingCountry],
                type: constant.COMMUNITY_TYPE.COUNTRY,
                createdAt: new Date()
            })

        }

        // now for each user check if they have the country community linked
        let users = await db_user.find()

        for (let user of users) {

            if (!user.profile.country) {
                console.log('MISSING COUNTRY', user._id)
                continue
            }

            // get the country's id
            let countryCommunity = await db_community.findOne({
                geolocation: user.profile.country,
                type: constant.COMMUNITY_TYPE.COUNTRY
            })

            // check if there is a country community that matches their country
            let userCountryCommunity = await db_user_community.findOne({
                userId: user._id,
                communityId: countryCommunity._id
            })

            if (!userCountryCommunity) {

                await db_user_community.save({
                    userId: user._id,
                    communityId: countryCommunity._id
                })

            }
        }
    } catch (err) {
        console.error(err)
    }

    process.exit(1)

})()
