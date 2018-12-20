import {Request, Response, NextFunction, Router} from 'express';
import {getEnv} from '../utility';
import db from '../db';
import {utilCrypto} from '../utility';
import * as moment from 'moment';

import test from './test';
import user from './user';
import team from './team';
import task from './task';
import taskCandidate from './task_candidate';
import teamCandidate from './team_candidate';
import community from './community';
import submission from './submission';
import cvote from './cvote';

import upload from './upload';

import ping from './ping';

/**
 * Every request intercepts the token and sets the session user from the userId again
 *
 * @param {e.Request} req
 * @param {e.Response} res
 * @param {e.NextFunction} next
 * @returns {boolean}
 */
export const middleware = (req: Request, res: Response, next: NextFunction) => {
    // check token
    const token = req.headers['api-token'];

    if (token) {
        try {
            const json = JSON.parse(utilCrypto.decrypt(token.toString()));
            if (json.userId && json.expired && (json.expired - moment().unix() > 0)) {
                db.create().then((DB) => {
                    DB.getModel('User').findOne({_id: json.userId}).then((user) => {

                        // TODO: find better way to not send the salt back to the front-end
                        delete user._doc.salt

                        if (user) {
                            req['session'].user = user;
                            req['session'].userId = user.id;
                        }

                        next();
                    }).catch(() => {
                        next();
                    })
                });
                return false;
            }
        } catch (e) {
            next();
        }

    } else if (req['session'].userId) {
        // check session
        const session = req['session'];
        db.create().then((DB) => {
            DB.getModel('User').findOne({_id: session.userId}).then((user) => {
                if (user) {
                    req['session'].user = user;
                }

                next();
            }).catch(() => {
                next();
            })
        });
        return false;
    }

    next();
};

const router = Router();

if (getEnv() === 'dev') {
    router.use('/test', test);
}

router.use('/ping', ping);

router.use('/user', user);
router.use('/team', team);
router.use('/task', task);
router.use('/taskCandidate', taskCandidate)
router.use('/teamCandidate', teamCandidate)
router.use('/community', community);
router.use('/upload', upload);
router.use('/submission', submission);
router.use('/cvote', cvote);

router.use((req, res) => {
    return res.sendStatus(403);
});

export default router;
