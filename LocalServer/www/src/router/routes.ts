import { RouteRecordRaw } from 'vue-router';

const routes: RouteRecordRaw[] = [
  {
    path: '/',
    component: () => import('layouts/MainLayout.vue'),
    children: [
      { path: '', component: () => import('pages/IndexPage.vue') },
      { path: '/player/remote/', component: () => import('src/pages/Player/RemoteCtrl.vue') },
      { path: '/media_lib/all/', component: () => import('src/pages/MediaLib/All.vue') },
      { path: '/media_lib/artist/', component: () => import('src/pages/MediaLib/Artist.vue') },
      { path: '/media_lib/album/', component: () => import('src/pages/MediaLib/Album.vue') },
    ]
  },

  // Always leave this as last one,
  // but you can also remove it
  {
    path: '/:catchAll(.*)*',
    component: () => import('pages/ErrorNotFound.vue'),
  },
];

export default routes;
